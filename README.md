# simulador-de-trafego
simulação concorrente de tráfego urbano em C, na qual veículos são representados por threads e competem por espaços de uma malha viária. O foco é aplicar conceitos de concorrência, exclusão mútua, espera bloqueante, semáforos, variáveis de condição e prevenção de deadlocks.

## Compilação e execução

```
make          # ou o comando de build equivalente do projeto
./simulador [numero_de_ticks] [tick_us]
```

- `numero_de_ticks` (opcional): quantidade de ticks a simular. Se omitido, roda indefinidamente até Ctrl+C.
- `tick_us` (opcional): duração de cada tick em microssegundos. Padrão: `700000` (0,7s). Valores menores tornam a simulação mais rápida (útil para testes longos/automatizados); valores maiores facilitam acompanhar visualmente cada movimento (útil para demonstração).

Exemplo para demonstração mais lenta e fácil de acompanhar:
```
./simulador 200 900000
```

## Legenda da visualização

- `R`/`M`/`L`: carro rápido / médio / lento
- `@`: ambulância
- `H`: semáforo verde para o eixo horizontal daquele cruzamento
- `V`: semáforo verde para o eixo vertical daquele cruzamento
- `X`: cruzamento fechado nos dois sentidos (transição segura entre fases)
- `██`: calçada/parede
