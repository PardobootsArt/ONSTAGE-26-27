# python & "C:\Users\Aluno\AppData\Local\Programs\Python\Python313\python.exe" "C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\mikefala.py"
import os
import json
import queue
import unicodedata

import sounddevice as sd
import pygame
from vosk import Model, KaldiRecognizer
import pygame

import socket


# ==============================
# CAMINHOS — ALTERE PARA OS SEUS
# ==============================
PASTA_AUDIOS = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\AUDIO"
CAMINHO_MODELO = r"C:\Users\Aluno\Documents\GitHub\ONSTAGE-26-27\NACIONAL\AUDIO\vosk-model-small-pt-0.3"

# ==============================
# PALAVRAS-CHAVE E ÁUDIOS DO MIKE
# ==============================

palavras_chave = {
    "gente": "rosto na TV.mp3",
    "mamae": "seboso.mp3",
    "gostando": "comovai.mp3",
    "oi": "Bom dia assustar.mp3",
    "cumprida": "comemoração.mp3"
}

# ==============================
# PREPARAÇÃO
# ==============================

fila_audio = queue.Queue()

def normalizar(texto):
    texto = texto.lower()
    texto = unicodedata.normalize("NFD", texto)
    texto = "".join(
        letra for letra in texto
        if unicodedata.category(letra) != "Mn"
    )
    return texto

def reproduzir_audio(nome_arquivo):
    caminho = os.path.join(PASTA_AUDIOS, nome_arquivo)

    if not os.path.isfile(caminho):
        print(f"Áudio não encontrado: {caminho}")
        return

    print(f"Reproduzindo: {nome_arquivo}")
    pygame.mixer.music.load(caminho)
    pygame.mixer.music.play()

    while pygame.mixer.music.get_busy():
        pygame.time.Clock().tick(10)

def callback(indata, frames, time_info, status):
    if status:
        print(status)

    fila_audio.put(bytes(indata))

# ==============================
# INICIALIZAÇÃO
# ==============================

if not os.path.isdir(CAMINHO_MODELO):
    raise FileNotFoundError(
        f"Modelo Vosk não encontrado: {CAMINHO_MODELO}"
    )

if not os.path.isdir(PASTA_AUDIOS):
    raise FileNotFoundError(
        f"Pasta de áudios não encontrada: {PASTA_AUDIOS}"
    )

for arquivo in palavras_chave.values():
    caminho = os.path.join(PASTA_AUDIOS, arquivo)
    if not os.path.isfile(caminho):
        print(f"ATENÇÃO: confira o nome do arquivo: {arquivo}")

pygame.mixer.init()

modelo = Model(CAMINHO_MODELO)
reconhecedor = KaldiRecognizer(modelo, 16000)

ultima_palavra = None

print("Mike está pronto para reconhecer as palavras!")
print("Palavras:", ", ".join(palavras_chave.keys()))

# ==============================
# CONEXÃO COM O UNITY
# ==============================

HOST = "127.0.0.1"
PORTA = 5005

servidor = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
servidor.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
servidor.bind((HOST, PORTA))
servidor.listen(1)

print("Aguardando conexão com o Unity...")
conexao, endereco = servidor.accept()

print("Unity conectado:", endereco)

# ==============================
# RECONHECIMENTO DE VOZ
# ==============================

with sd.RawInputStream(
    samplerate=16000,
    blocksize=8000,
    dtype="int16",
    channels=1,
    callback=callback
):
    while True:
        dados = fila_audio.get()

        if reconhecedor.AcceptWaveform(dados):
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

            elif palavra_encontrada != ultima_palavra:
                arquivo = palavras_chave[palavra_encontrada]
                reproduzir_audio(arquivo)
                # Envia somente a palavra-chave ao Unity
                mensagem = f"PALAVRA:{palavra_encontrada}\n"
                conexao.sendall(mensagem.encode("utf-8"))

                print("Enviado ao Unity:", mensagem.strip())
                ultima_palavra = palavra_encontrada
