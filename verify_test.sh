#!/usr/bin/env bash
# verify_test.sh - compiles and tests each source file in the repo before committing.
# Warnings count as failures. Binaries go to a tmp directory, keeping the repo clean.
# Usage: bash verify_test.sh

set -u

failures=0
tmp="$(mktemp -d)"

trap 'rm -rf "$tmp"' EXIT

ok() {
	printf "  \033[32mOK\033[0m   %s\n" "$1"
}

fail() {
	printf "  \033[31mFAIL\033[0m %s\n" "$1"
	failures=$((failures + 1))
}

compile_c() {
	# $1 = source files
	# $2 = output name
	# $3 = extra flags, for example: -lm

	local sources="$1"
	local output_name="$2"
	local extra_flags="${3:-}"
	local out="$tmp/$output_name"

	if gcc -O2 -Wall -Wextra $sources -o "$out" $extra_flags 2>"$tmp/err"; then
		if [ -s "$tmp/err" ]; then
			fail "$output_name (warnings)"
			sed 's/^/      /' "$tmp/err"
		else
			ok "$output_name"
		fi
	else
		fail "$output_name (does not compile)"
		sed 's/^/      /' "$tmp/err"
	fi
}

echo "== Compiling C sources =="

[ -f dijkstra.c ]            && compile_c "dijkstra.c"             "dijkstra"          ""
[ -f astar.c ]               && compile_c "astar.c"                "astar"             "-lm"
[ -f astar_municipios_pi.c ] && compile_c "astar_municipios_pi.c" "astar_municipios"  "-lm"
[ -f blockchain.c ] && [ -f sha256.c ] && compile_c "blockchain.c sha256.c" "blockchain_demo" ""
[ -f semiring_path.c ]       && compile_c "semiring_path.c"       "semiring_path"     ""
[ -f warshall.c ]            && compile_c "warshall.c"            "wharshall"         ""
[ -f markov.c ]              && compile_c "markov.c"              "markov"            "-lm"
[ -f fiscal_otimizador.c ]   && compile_c "fiscal_otimizador.c"   "fiscal_otimizador" "-lm"

if [ -f Pointers_C++.cpp ]; then
	if g++ -O2 -Wall -Wextra Pointers_C++.cpp -o "$tmp/pointers" 2>"$tmp/err"; then
		if [ -s "$tmp/err" ]; then
			fail "Pointers_C++.cpp (warnings)"
			sed 's/^/      /' "$tmp/err"
		else
			ok "Pointers_C++.cpp"
		fi
	else
		fail "Pointers_C++.cpp"
		sed 's/^/      /' "$tmp/err"
	fi
fi

echo "== Smoke tests (execution) =="

[ -x "$tmp/dijkstra" ] && {
	"$tmp/dijkstra" Deposito Cliente >/dev/null &&
		ok "dijkstra runs" ||
		fail "dijkstra runtime"
}

[ -x "$tmp/blockchain_demo" ] && {
	"$tmp/blockchain_demo" >/dev/null &&
		ok "blockchain runs" ||
		fail "blockchain runtime"
}

[ -x "$tmp/semiring_path" ] && {
	"$tmp/semiring_path" >/dev/null &&
		ok "semiring runs" ||
		fail "semiring runtime"
}

[ -x "$tmp/wharshall" ] && {
	"$tmp/wharshall" >/dev/null &&
		ok "warshall runs" ||
		fail "warshall runtime"
}

if [ -x "$tmp/astar_municipios" ] && [ -f nos.csv ] && [ -f arestas.csv ]; then
	source_node="$(awk -F, 'NR==2{print $1; exit}' nos.csv)"
	target_node="$(awk -F, 'END{print $1}' nos.csv)"

	"$tmp/astar_municipios" "$source_node" "$target_node" nos.csv arestas.csv >/dev/null 2>&1 &&
		ok "astar_municipios runs ($source_node -> $target_node)" ||
		fail "astar_municipios runtime"
fi

echo

if [ "$failures" -eq 0 ]; then
	printf "\033[32mALL OK\033[0m - safe to commit.\n"
	exit 0
else
	printf "\033[31m%d failure(s)\033[0m - fix before committing.\n" "$failures"
	exit 1
fi
