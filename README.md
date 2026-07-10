# Simulador de Tráfego Urbano em C

Simulação concorrente de tráfego urbano em C, na qual veículos são representados por
threads e competem por espaços de uma malha viária. O projeto aplica conceitos de
concorrência, exclusão mútua, espera bloqueante, semáforos, variáveis de condição e
prevenção de deadlocks — trabalho da disciplina de Sistemas Operacionais (Engenharia de
Software, UFCA).

## Integrantes

- Ângelo Gabriel Alves Freire Duarte
- Jetro Kepler Gomes Alencar Gonzaga Viana
- José Luiz de Lima Mendes

## Visão geral

- Malha viária representada por uma matriz de 21 linhas × 55 colunas, com 8 cruzamentos,
  ruas contínuas (horizontais e verticais) e pelo menos uma via de mão única.
- 14 carros + 1 ambulância rodando simultaneamente, cada um como uma thread própria.
- Relógio global discreto (thread separada) que avança em ticks e acorda as demais threads.
- 8 threads de semáforo (uma por cruzamento), com sincronização por mutex, variável de
  condição e semáforo de contagem.
- Ambulância com prioridade de passagem nos cruzamentos, sem violar a exclusão mútua das
  células.
- Visualização em tempo real no terminal (ASCII + cores ANSI), atualizada a cada tick.
- Log de eventos em `simulation.log`.

## Requisitos

- Compilador `gcc` com suporte a C11 (ou superior) e à biblioteca `pthread`.
- Sistema Linux/Unix com terminal ANSI (para as cores da visualização).

## Compilação

O projeto **não usa Makefile**. A compilação é feita diretamente com o comando abaixo,
executado na raiz do repositório:

```bash
gcc -Wall -Wextra -o simulator main.c map.c render.c clock.c traffic_light.c vehicle.c ambulance.c logger.c -lpthread
```

- `-Wall -Wextra`: habilita avisos adicionais do compilador, ajudando a identificar
  problemas de concorrência e uso incorreto de variáveis ainda em tempo de compilação.
- `-lpthread`: liga a biblioteca de threads POSIX, obrigatória para `pthread_create`,
  mutexes, variáveis de condição e semáforos usados no projeto.
- `-o simulator`: gera o executável com o nome `simulator`.

Se a compilação for bem-sucedida, nenhum erro nem aviso deve aparecer no terminal e o
binário `simulator` será criado na raiz do projeto.

## Execução

```bash
./simulator [numero_de_ticks] [tick_us]
```

Ambos os argumentos são opcionais e posicionais:

| Argumento | Obrigatório | Padrão | Descrição |
|---|---|---|---|
| `numero_de_ticks` | Não | roda indefinidamente até `Ctrl+C` | Quantidade de ticks a simular. Deve ser um inteiro maior que 0. |
| `tick_us` | Não | `700000` (0,7s) | Duração de cada tick em microssegundos. Valores menores tornam a simulação mais rápida (útil para testes automatizados); valores maiores facilitam acompanhar visualmente cada movimento. |

Exemplos:

```bash
# Roda indefinidamente, com o tick padrão (0,7s), até Ctrl+C
./simulator

# Roda exatamente 200 ticks, com o tick padrão
./simulator 200

# Roda 200 ticks com tick de 0,9s — mais lento, melhor para demonstração
./simulator 200 900000

# Roda 500 ticks com tick de 100ms — mais rápido, útil para testes longos
./simulator 500 100000
```

Para encerrar a simulação antes do fim (quando `numero_de_ticks` não é informado), use
`Ctrl+C`. O programa finaliza todas as threads de forma segura antes de encerrar.

## Legenda da visualização

| Símbolo | Significado |
|---|---|
| `R` / `M` / `L` | Carro rápido / médio / lento |
| `@` | Ambulância |
| `H` | Semáforo verde para o eixo **horizontal** daquele cruzamento (eixo vertical em vermelho) |
| `V` | Semáforo verde para o eixo **vertical** daquele cruzamento (eixo horizontal em vermelho) |
| `X` | Cruzamento fechado nos dois sentidos (transição segura entre fases) |
| `██` | Calçada / parede |
| `→ ↑ ← ↓` | Via de rolamento e sentido permitido de movimento naquela célula |

> Cada célula de cruzamento representa **dois eixos ao mesmo tempo** (horizontal e
> vertical). Por isso não existe um símbolo isolado de "vermelho": o vermelho de um eixo
> está implícito sempre que o outro eixo aparece com `H` ou `V`, e `X` indica os dois
> eixos fechados simultaneamente (fase de transição, quando nenhum veículo pode atravessar).

## Estrutura do projeto

| Arquivo | Responsabilidade |
|---|---|
| `map.c` / `map.h` | Estruturas do mapa (`Map`, `Cell`, `CellType`, `Direction`) e inicialização da malha viária com 8 cruzamentos. |
| `render.c` / `render.h` | Renderização ASCII/ANSI da malha, veículos e semáforos a cada tick. |
| `clock.c` / `clock.h` | Relógio global discreto (`GlobalClock`), thread do relógio e `wait_until_tick`. |
| `traffic_light.c` / `traffic_light.h` | Threads de semáforo (uma por cruzamento), alternância de fase e liberação de veículos bloqueados. |
| `vehicle.c` / `vehicle.h` | Threads de veículo, cálculo de rotas, movimentação e exclusão mútua das células. |
| `ambulance.c` / `ambulance.h` | Lógica de prioridade da ambulância nos cruzamentos. |
| `logger.c` / `logger.h` | Log thread-safe de eventos em `simulation.log`. |
| `main.c` | Inicialização geral, criação das 24 threads e laço principal de simulação. |

## Sincronização (resumo)

- **Mutex** (`cell->lock`, `tl->lock`, `g_clock.lock`): protege o estado de cada célula do
  mapa, o estado de cada semáforo e o contador de tick global.
- **Variável de condição** (`g_clock.cond`, `tl->cond_horiz`, `tl->cond_vert`): acorda
  threads bloqueadas (veículos aguardando o próximo tick ou aguardando sinal verde) sem uso
  de espera ocupada.
- **Semáforo de contagem** (`tl->entry_sem`): limita quantos veículos podem ocupar
  simultaneamente o bloco de um cruzamento.
- **Prevenção de deadlock**: ao mover um veículo entre duas células, os mutexes são sempre
  adquiridos em ordem fixa (menor índice de célula primeiro) e o segundo lock usa
  `pthread_mutex_trylock`, evitando espera circular e *hold-and-wait*.

Detalhes completos de cada mecanismo estão no relatório técnico do projeto.

## Log de eventos

Durante a execução, eventos relevantes (movimentos inválidos, bloqueios por célula
ocupada, solicitação/liberação de prioridade da ambulância, mudanças de fase de semáforo
etc.) são registrados em `simulation.log`, na raiz do projeto.

## Repositório

<https://github.com/jetrokepler/simulador-de-trafego>
