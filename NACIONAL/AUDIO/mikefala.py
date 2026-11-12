# & "C:\Users\Aluno\AppData\Local\Programs\Python\Python313\python.exe" "C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\mikefala.py"

import os
import json
import queue
import unicodedata
import socket
import threading


import sounddevice as sd
import pygame
from vosk import Model, KaldiRecognizer


# ==========================================
# CAMINHOS
# ==========================================

PASTA_AUDIOS = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\AUDIO"

CAMINHO_MODELO = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\vosk-model-small-pt-0.3"


# ==========================================
# PALAVRAS-CHAVE, NÚMEROS E ÁUDIOS
# ==========================================

palavras_chave = {

    "primeiro": {
        "audio": "semsom_.mp3",
        "numero": 1
    },
    "hoje": {
        "audio": "rosto na TV.mp3",
        "numero": 2
    },
    "mamae": {
        "audio": "ofende, mas não assassina o português.mp3",
        "numero": 3
        
    },
     "bravo": {
        "audio": "seboso.mp3",
        "numero": 4   
    },
    "galera": {
        "audio": "ze mané.mp3",
        "numero": 5
    },
    "respeito": {
        "audio": "bom dia assustar.mp3",
        "numero": 6
    },
    "artista": {
        "audio": "tudo bem ta na hora.mp3",
        "numero": 7
    },
    "objetivo": {
        "audio": "semsom_.mp3",
        "numero": 8
    },
     "cracha": {
        "audio": "semsom_.mp3",
        "numero": 9
    },
     "trabalho": {
        "audio": "semsom_.mp3",
        "numero": 10
    },    
   
    "habilidades": {
        "audio": "semsom_.mp3",
        "numero": 11
    },
    "forte": {
        "audio": "semsom_.mp3",
        "numero": 12
    },
    "funcionou": {
        "audio": "semsom_.mp3",
        "numero": 13
    },
    "cumprida": {
        "audio": "comemoração.mp3",
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


def enviar_comandos():
    global conexao

    while True:
        mensagem = fila_comandos.get()

        while True:
            # Aguarda uma conexão sem bloquear o reconhecimento
            evento_conexao.wait()

            with conexao_lock:
                socket_atual = conexao

            if socket_atual is None:
                evento_conexao.clear()
                continue

            try:
                socket_atual.sendall(mensagem.encode("utf-8"))

                print(f"Comando enviado ao Unity: {mensagem.strip()}")
                break

            except OSError as erro:
                print(f"Conexão perdida: {erro}")

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
reconhecedor = KaldiRecognizer(modelo, 16000)

fila_audio = queue.Queue()


def callback(indata, frames, time_info, status):
    if status:
        print(f"Áudio: {status}")

    fila_audio.put(bytes(indata))


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
# ==========================================

ultima_palavra = None

try:
    with sd.RawInputStream(
        samplerate=16000,
        blocksize=8000,
        dtype="int16",
        channels=1,
        callback=callback
    ):
        while True:
            dados = fila_audio.get()

            if not reconhecedor.AcceptWaveform(dados):
                continue

            resultado = json.loads(reconhecedor.Result())
            fala = normalizar(resultado.get("text", ""))

            if not fala:
                ultima_palavra = None
                continue

            print("Você disse:", fala)

            palavra_encontrada = None

            for palavra in palavras_chave:
                if palavra in fala.split():
                    palavra_encontrada = palavra
                    break

            if palavra_encontrada is None:
                ultima_palavra = None
                continue

            if palavra_encontrada == ultima_palavra:
                continue

            dados_palavra = palavras_chave[palavra_encontrada]

            numero = dados_palavra["numero"]
            arquivo = dados_palavra["audio"]

            # 1. Coloca o comando na fila de envio ao Unity
            mensagem = f"PALAVRA:{numero}\n"
            fila_comandos.put(mensagem)
            print("COMANDO COLOCADO NA FILA:", repr(mensagem))

            print(f"Comando preparado para o Unity: {numero}")

            # 2. Reproduz o áudio depois de enfileirar o comando
            if arquivo is not None:
                reproduzir_audio(arquivo)

            print(f"Palavra reconhecida: {palavra_encontrada}")

            ultima_palavra = palavra_encontrada

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

    pygame.mixer.quit()
    print("Programa finalizado.")