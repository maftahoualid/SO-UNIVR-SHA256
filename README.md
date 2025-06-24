# SHA-256 Client-Server System 
### (Progetto didattico per il corso di **Sistemi Operativi** – Università di Verona)

## Descrizione

Questo progetto implementa un sistema client-server in C che utilizza le primitive IPC di System V per calcolare l'hash SHA-256 del contenuto di file.  
Il server è multi-processo, gestisce richieste concorrenti da parte dei client e supporta comandi di controllo (modifica del numero massimo di figli attivi) e interrogazione dello stato.

## Caratteristiche

- ✅ Calcolo SHA-256 con OpenSSL
- ✅ Comunicazione via:
  - Coda di messaggi (System V)
  - Memoria condivisa (System V)
  - Semafori (System V)
- ✅ Controllo concorrente tramite semaforo POSIX
- ✅ Supporto a comandi di:
  - Modifica limite figli (`client_control`)
  - Stato server (`client_status`)
- ✅ Gestione corretta di segnali (`SIGINT`, `SIGCHLD`)
- ✅ Pulizia automatica delle risorse IPC

## Requisiti

- **CMake ≥ 3.20**
- **OpenSSL** (es: `libssl-dev` su Debian/Ubuntu)
- **Sistema Unix-like** (Linux o macOS)

## Compilazione

1. Clonare il progetto:
   ```bash
   git clone <url-del-repo>
   cd sha256-project
   ```

2. Costruire con CMake:
   ```bash
   mkdir build && cd build
   cmake ..
   make
   ```

## Esecuzione

### Avvio del server
```bash
./server
```

### Richiesta di hash (SHA-256) da file
```bash
./client path/al/file
```

### Modifica del numero massimo di processi figli
```bash
./client_control <nuovo_limite>
```

### Richiesta stato server
```bash
./client_status
```

### Arresto server 
```bash
./client_shutdown
```

## Struttura del progetto

```
.
├── include/
│   ├── common.h
│   └── digest.h
├── test
    ├── file1.txt
    └── file2.txt
├── src/
│   ├── client.c
│   ├── server.c
│   ├── digest.c
│   ├── client_control.c
│   ├── client_status.c
│   └── client_shutdown.c
├── CMakeLists.txt
└── README.md
```

## Esempio di output

```bash
$ ./client file1.txt
SHA-256: 4a44dc15364204a80fe80e9039455cc1608281820fe2b24ad48e3be01f0f1e02

$ ./client_status
[client_status] Stato server:
 - Processi figli attivi: 1
 - Limite massimo:        4

$ ./client_control 2
[client_control] Nuovo limite richiesto: 2
```
