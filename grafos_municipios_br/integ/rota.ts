/*
 * rota.ts - Contratos emitidos pelos motores C, + validador de runtime.
 *   - RotaAstar: saida de `astar_br --json`  (ponto-a-ponto)
 *   - RotaTsp:   saida de `tsp_br --json`     (roteiro otimizado)
 */
export interface RotaAstar {
  origem:  { code: string; nome: string; uf: string };
  destino: { code: string; nome: string; uf: string };
  km_total: number | null;
  saltos: number;
  estados: string[];
  expansao: { astar: number; dijkstra: number };
  caminho: string[];
  classificacao: string;
  erro?: string;
}

export interface RotaTsp {
  tipo: "tsp";
  fechado: boolean;
  km_total: number;
  km_inicial_nn: number;
  reducao_percent: number;
  paradas: { code: string; nome: string; uf: string }[];
  legs: { de: string; para: string; km: number; saltos: number }[];
  caminho_completo: string[];
  classificacao: string;
  erro?: string;
}

export type Rota = RotaAstar | RotaTsp;

export function isTsp(r: Rota): r is RotaTsp {
  return (r as RotaTsp).tipo === "tsp";
}

export function parseRota(raw: string): Rota {
  let o: any;
  try {
    o = JSON.parse(raw);
  } catch {
    throw new Error("stdin nao e JSON valido (esperado: saida de --json de um motor C).");
  }
  // JSON de erro generico (astar: sem origem; tsp: sem paradas)
  if (o && typeof o.erro === "string" && !o.origem && !o.paradas) {
    throw new Error(`motor C retornou erro: ${o.erro}`);
  }
  if (o && o.tipo === "tsp") {
    for (const campo of ["paradas", "legs", "caminho_completo", "km_total"]) {
      if (!(campo in o)) throw new Error(`contrato TSP invalido: falta campo "${campo}".`);
    }
    return o as RotaTsp;
  }
  for (const campo of ["origem", "destino", "caminho", "estados", "expansao"]) {
    if (!(campo in o)) throw new Error(`contrato invalido: falta campo "${campo}".`);
  }
  return o as RotaAstar;
}
