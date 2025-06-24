cmake_minimum_required(VERSION 3.20)  # Richiede almeno CMake 3.20

project(SHA256 VERSION 1.0)            # Nome progetto e versione

# Impostazioni generali
set(CMAKE_MACOSX_RPATH 1)              # Abilita RPATH su macOS (per ricerca librerie)
set(BUILD_SHARED_LIBS ON)              # Compila librerie come shared (se ci fossero)
set(CMAKE_POSITION_INDEPENDENT_CODE ON) # Compila con PIC (utile per shared libs)

# Opzioni di compilazione: disabilita warning su funzioni deprecated
add_compile_options(-Wno-deprecated-declarations)

# Trova libreria OpenSSL (necessaria per SHA-256)
find_package(OpenSSL REQUIRED)

# Include directory: cartella include e include OpenSSL, marcate come SYSTEM per ignorare warning
include_directories(SYSTEM ${PROJECT_SOURCE_DIR}/include ${OPENSSL_INCLUDE_DIR})

# --- Eseguibili ---

# Client: solo src/client.c, linka OpenSSL
add_executable(client src/client.c)
target_link_libraries(client ${OPENSSL_LIBRARIES})

# Server: src/server.c e src/digest.c, linka OpenSSL
add_executable(server src/server.c src/digest.c)
target_link_libraries(server ${OPENSSL_LIBRARIES})

# Altri client (controllo, status, shutdown) non linkati con OpenSSL (presumibilmente non usano digest)
add_executable(client_control src/client_control.c)
add_executable(client_status src/client_status.c)
add_executable(client_shutdown src/client_shutdown.c)
