#!/usr/bin/env bash
# setup_check.sh - diagnostico do pipeline C -> Node (rode da RAIZ do projeto).
# Os codigos de teste sao derivados do proprio data/nos_br.csv -> funciona
# tanto no sample sintetico quanto na base nacional real.
set -u
cd "$(dirname "$0")/.." || exit 1
ok(){ echo "[OK] $1"; }; bad(){ echo "[FALHA] $1"; }

NOS=data/nos_br.csv
if [ ! -f "$NOS" ]; then bad "nao achei $NOS (rode da raiz do projeto)"; exit 1; fi

# dois codigos REAIS do dataset (campo 1, pulando o header)
C1=$(awk -F, 'NR==2{gsub(/"/,"",$1); print $1; exit}' "$NOS")
C2=$(awk -F, 'NR==3{gsub(/"/,"",$1); print $1; exit}' "$NOS")
echo "    (codigos de teste extraidos do dataset: $C1 e $C2)"

# 1) astar_br tem --json e emite JSON valido com codigos reais?
if ./astar_br --json "$C1" "$C2" 2>/dev/null | head -c1 | grep -q '{'; then
  ok "astar_br emite JSON (--json presente)"
else
  bad "astar_br NAO emite JSON -> salve o src/astar_br.c novo e: make clean && make"
fi

# 2) deps do integ instaladas?
TSX=integ/node_modules/.bin/tsx
[ -x "$TSX" ] && ok "tsx local instalado" || bad "tsx local ausente -> cd integ && npm install"

# 3) pipe dry-run nos dois clients (sem chave/rede)
if [ -x "$TSX" ]; then
  ./astar_br --json "$C1" "$C2" 2>/dev/null | "$TSX" integ/anthropic.ts --dry-run >/dev/null 2>&1 \
    && ok "pipe C -> anthropic.ts" || bad "pipe C -> anthropic.ts"
  ./astar_br --json "$C1" "$C2" 2>/dev/null | "$TSX" integ/openai.js --dry-run >/dev/null 2>&1 \
    && ok "pipe C -> openai.js" || bad "pipe C -> openai.js"
fi
