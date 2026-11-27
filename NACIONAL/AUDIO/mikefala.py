# & "C:\Users\Aluno\AppData\Local\Programs\Python\Python313\python.exe" "C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\mikefala.py"

import os
import json
import queue
import unicodedata
import socket
import threading
import serial
import time

import sounddevice as sd
import pygame
from vosk import Model, KaldiRecognizer


# ==========================================
# CAMINHOS
# ==========================================

PASTA_AUDIOS = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\AUDIO"

CAMINHO_MODELO = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\vosk-model-small-pt-0.3"

# ==========================================
# AJUSTES DO RECONHECIMENTO
# ==========================================

# Tamanho de cada pedaço de áudio. Menor = reage mais rápido.
# 4000 amostras = 0,25 s (antes era 8000 = 0,5 s)
TAMANHO_BLOCO = 4000

# Depois que o Mike fala, joga fora o que o microfone ouviu durante a fala
# (evita ele "ouvir a própria voz" e disparar de novo).
IGNORAR_AUDIO_DURANTE_FALA = True

# Opcional: faz o Vosk procurar SÓ as palavras-chave (o resto vira [unk]).
# Deixa o reconhecimento das palavras mais rápido e preciso.
# Se alguma palavra parar de ser reconhecida, volte para False.
USAR_GRAMATICA = False

# Palavras escritas COM acento, do jeito que estão no modelo do Vosk
# (só é usado se USAR_GRAMATICA = True)
PALAVRAS_GRAMATICA = [
    "primeiro", "estrela", "mamãe", "comigo", "galera", "respeito",
    "artista", "objetivo", "crachá", "ativada", "habilidades",
    "gritar", "funcionou", "cumprida",
]

# ==========================================
# CONFIGURAÇÃO DO ARDUINO
# ==========================================

PORTA_ARDUINO = "COM7"  # Altere para a porta do seu Mega
BAUD_ARDUINO = 9600

arduino = None

try:
    arduino = serial.Serial(
        PORTA_ARDUINO,
        BAUD_ARDUINO,
        timeout=1
    )

    # O Mega pode reiniciar quando a conexão serial é aberta
    time.sleep(2)

    print(f"Arduino conectado na porta {PORTA_ARDUINO}!")

except serial.SerialException as erro:
    print(f"ERRO ao conectar com o Arduino: {erro}")


# ==========================================
# PALAVRAS-CHAVE, NÚMEROS E ÁUDIOS
# ==========================================

palavras_chave = {

    "primeiro": {
        "audio": "semsom_.mp3",
        "numero": 1
    },
    "estrela": {
        "audio": "rosto na TV.mp3",
        "numero": 2
    },
    "mae": {
        "audio": "ofende, mas não assassina o português.mp3",
        "numero": 3
    },
    "comigo": {
        "audio": "seboso.mp3",
        "numero": 4
    },
    "artista": {
        "audio": "semsom_.mp3",
        "numero": 7
    },
    "objetivo": {
        "audio": "semsom_.mp3",
        "numero": 8
    },
    "ativada": {
        "audio": "semsom_.mp3",
        "numero": 10
    },
    "habilidades": {
        "audio": "semsom_.mp3",
        "numero": 11
    },
    "tres": {
        "audio": "semsom_.mp3",
        "numero": 12
    },
    "funcionou": {
        "audio": "semsom_.mp3",
        "numero": 13
    },
    "cumprida": {
        "audio": "semsom_.mp3",
        "numero": 14
    }
}


# ==========================================
# CONFIGURAÇÃO DO UNITY
# ==========================================

HOST = "127.0.0.1"
PORTA = 5005

servidor = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
servidor.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
servidor.bind((HOST, PORTA))
servidor.listen(5)

conexao = None
conexao_lock = threading.Lock()
evento_conexao = threading.Event()
fila_comandos = queue.Queue()


# ==========================================
# NORMALIZAÇÃO DA FALA
# ==========================================

def normalizar(texto):
    texto = texto.lower()
    texto = unicodedata.normalize("NFD", texto)

    texto = "".join(
        letra
        for letra in texto
        if unicodedata.category(letra) != "Mn"
    )

    return texto


# ==========================================
# REPRODUÇÃO DOS ÁUDIOS
# ==========================================

def reproduzir_audio(nome_arquivo):
    if nome_arquivo is None:
        return

    caminho = os.path.join(PASTA_AUDIOS, nome_arquivo)

    if not os.path.isfile(caminho):
        print(f"Áudio não encontrado: {caminho}")
        return

    try:
        print(f"Reproduzindo: {nome_arquivo}")

        pygame.mixer.music.load(caminho)
        pygame.mixer.music.play()

        while pygame.mixer.music.get_busy():
            pygame.time.Clock().tick(20)

    except pygame.error as erro:
        print(f"Erro ao reproduzir áudio: {erro}")


# ==========================================
# CONEXÃO COM O UNITY
# ==========================================

def aceitar_conexoes():
    global conexao

    while True:
        try:
            nova_conexao, endereco = servidor.accept()

            nova_conexao.setsockopt(
                socket.IPPROTO_TCP,
                socket.TCP_NODELAY,
                1
            )

            with conexao_lock:
                conexao_antiga = conexao
                conexao = nova_conexao
                evento_conexao.set()

            if conexao_antiga is not None:
                try:
                    conexao_antiga.close()
                except OSError:
                    pass

            print(f"Unity conectado: {endereco}")

        except OSError as erro:
            print(f"Erro ao aceitar conexão: {erro}")


def receber_linha(sock):
    dados = bytearray()

    while True:
        parte = sock.recv(1)

        if not parte:
            raise ConnectionError("Unity encerrou a conexão.")

        if parte == b"\n":
            return dados.decode("utf-8").strip()

        dados.extend(parte)


def enviar_comandos():
    global conexao

    while True:
        mensagem = fila_comandos.get()

        while True:
            evento_conexao.wait()

            with conexao_lock:
                socket_atual = conexao

            if socket_atual is None:
                evento_conexao.clear()
                continue

            try:
                # 1. Envia o comando ao Unity
                socket_atual.sendall(mensagem.encode("utf-8"))

                print(f"Comando enviado ao Unity: {mensagem.strip()}")

                # 2. Aguarda a confirmação do Unity
                socket_atual.settimeout(10)

                resposta = receber_linha(socket_atual)

                numero = int(mensagem.split(":")[1].strip())

                if resposta == f"RECEBIDO:{numero}":

                    print("Unity confirmou:", resposta)

                    # 3. Só envia ao Arduino depois da confirmação
                    if numero == 11 or numero == 12:
                        enviar_arduino(numero)

                    socket_atual.settimeout(None)
                    break

                else:
                    print("Confirmação inesperada do Unity:", resposta)
                    socket_atual.settimeout(None)
                    break

            except OSError as erro:
                print(f"Conexão perdida ou sem confirmação: {erro}")

                with conexao_lock:
                    if conexao is socket_atual:
                        conexao = None
                        evento_conexao.clear()

                        try:
                            socket_atual.close()
                        except OSError:
                            pass

                print("Aguardando o Unity se reconectar...")


# ==========================================
# ENVIO DE COMANDOS AO ARDUINO
# ==========================================

def enviar_arduino(comando):
    if arduino is None or not arduino.is_open:
        print("Arduino não está conectado!")
        return

    try:
        # Limpa respostas antigas
        arduino.reset_input_buffer()

        # Envia o comando
        mensagem = f"{comando}\n"
        arduino.write(mensagem.encode("utf-8"))

        print(f"Enviado ao Arduino: {comando}")

        # Aguarda a confirmação
        resposta = arduino.readline().decode("utf-8").strip()

        if resposta:
            print("Arduino confirmou:", resposta)
        else:
            print("Arduino não respondeu!")

    except serial.SerialException as erro:
        print("Erro na comunicação:", erro)


# ==========================================
# VERIFICAÇÃO DOS ARQUIVOS
# ==========================================

if not os.path.isdir(CAMINHO_MODELO):
    raise FileNotFoundError(
        f"Modelo Vosk não encontrado: {CAMINHO_MODELO}"
    )

if not os.path.isdir(PASTA_AUDIOS):
    raise FileNotFoundError(
        f"Pasta de áudios não encontrada: {PASTA_AUDIOS}"
    )

for dados_palavra in palavras_chave.values():
    arquivo = dados_palavra["audio"]

    if arquivo is not None:
        caminho = os.path.join(PASTA_AUDIOS, arquivo)

        if not os.path.isfile(caminho):
            print(f"ATENÇÃO: áudio não encontrado: {arquivo}")


# ==========================================
# INICIALIZAÇÃO DO VOSK E DO ÁUDIO
# ==========================================

pygame.mixer.init()

modelo = Model(CAMINHO_MODELO)

if USAR_GRAMATICA:
    gramatica = json.dumps(PALAVRAS_GRAMATICA + ["[unk]"], ensure_ascii=False)
    reconhecedor = KaldiRecognizer(modelo, 16000, gramatica)
    print("Modo gramática: procurando só as palavras-chave.")
else:
    reconhecedor = KaldiRecognizer(modelo, 16000)

fila_audio = queue.Queue()


def callback(indata, frames, time_info, status):
    if status:
        print(f"Áudio: {status}")

    fila_audio.put(bytes(indata))


def limpar_audio_acumulado():
    """Descarta o áudio que chegou enquanto o Mike estava falando."""
    while True:
        try:
            fila_audio.get_nowait()
        except queue.Empty:
            break
    reconhecedor.Reset()


# ==========================================
# DISPARO DE UMA PALAVRA
# ==========================================

def disparar_palavra(palavra):
    dados_palavra = palavras_chave[palavra]

    numero = dados_palavra["numero"]
    arquivo = dados_palavra["audio"]

    print(f">>> Palavra reconhecida: {palavra}")

    # 1. Coloca o comando na fila de envio ao Unity
    mensagem = f"PALAVRA:{numero}\n"
    fila_comandos.put(mensagem)
    print("COMANDO COLOCADO NA FILA:", repr(mensagem))

    # 2. Reproduz o áudio depois de enfileirar o comando
    if arquivo is not None:
        reproduzir_audio(arquivo)


# ==========================================
# INICIAR AS THREADS DE REDE
# ==========================================

threading.Thread(
    target=aceitar_conexoes,
    daemon=True
).start()

threading.Thread(
    target=enviar_comandos,
    daemon=True
).start()

print("Mike está pronto para reconhecer as palavras!")
print("Palavras:", ", ".join(palavras_chave.keys()))
print(f"Servidor aguardando Unity na porta {PORTA}...")


# ==========================================
# RECONHECIMENTO CONTÍNUO DE VOZ
# (agora reconhece no MEIO da frase)
# ==========================================

ultima_palavra = None        # última palavra disparada na frase ANTERIOR
disparadas_na_frase = set()  # palavras já disparadas na frase ATUAL
parcial_anterior = []        # palavras do resultado parcial anterior

try:
    with sd.RawInputStream(
        samplerate=16000,
        blocksize=TAMANHO_BLOCO,
        dtype="int16",
        channels=1,
        callback=callback
    ):
        while True:
            dados = fila_audio.get()

            frase_terminou = reconhecedor.AcceptWaveform(dados)

            if frase_terminou:
                # ----- Frase completa (resultado final) -----
                resultado = json.loads(reconhecedor.Result())
                palavras = normalizar(resultado.get("text", "")).split()

                if palavras:
                    print("Você disse:", " ".join(palavras))

                # No resultado final todas as palavras são confiáveis
                candidatas = palavras

            else:
                # ----- Frase ainda em andamento (resultado parcial) -----
                resultado = json.loads(reconhecedor.PartialResult())
                palavras = normalizar(resultado.get("partial", "")).split()

                if palavras and palavras != parcial_anterior:
                    print("  ouvindo:", " ".join(palavras))

                # A última palavra do parcial ainda pode mudar
                # (ex.: "prime..." virar "primeira"). Só confia nela
                # se ela já apareceu no parcial anterior.
                candidatas = palavras[:-1]
                if palavras and palavras[-1] in parcial_anterior:
                    candidatas = palavras

                parcial_anterior = palavras

            # ----- Procura palavra-chave ainda não disparada -----
            palavra_disparada = None

            for palavra in candidatas:
                if (palavra in palavras_chave
                        and palavra not in disparadas_na_frase
                        and palavra != ultima_palavra):
                    palavra_disparada = palavra
                    break

            if palavra_disparada is not None:
                disparadas_na_frase.add(palavra_disparada)
                disparar_palavra(palavra_disparada)

                if IGNORAR_AUDIO_DURANTE_FALA:
                    # Começa uma "frase nova" depois que o Mike falou
                    limpar_audio_acumulado()
                    ultima_palavra = palavra_disparada
                    disparadas_na_frase = set()
                    parcial_anterior = []
                continue

            if frase_terminou:
                # Frase acabou sem palavra nova: libera repetir a última
                if not disparadas_na_frase:
                    ultima_palavra = None
                else:
                    ultima_palavra = list(disparadas_na_frase)[-1]

                disparadas_na_frase = set()
                parcial_anterior = []

except KeyboardInterrupt:
    print("\nPrograma encerrado pelo usuário.")

finally:
    with conexao_lock:
        if conexao is not None:
            try:
                conexao.close()
            except OSError:
                pass

    try:
        servidor.close()
    except OSError:
        pass

    if arduino is not None and arduino.is_open:
        arduino.close()
    print("Conexão com Arduino encerrada.")

    pygame.mixer.quit()
    print("Programa finalizado.")