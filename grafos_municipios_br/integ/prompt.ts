/*
 * prompt.ts - Monta o prompt a partir do contrato (astar OU tsp).
 *
 * Principio central: o C e o ORACULO; o LLM e o NARRADOR.
 * A selecao do que citar e DETERMINISTICA (codigo, nao o modelo).
 */
import type { Rota, RotaAstar, RotaTsp } from "./rota.ts";
import { isTsp } from "./rota.ts";

export interface Prompt {
  system: string;
  user: string;
}

const MAX_LISTA = 12;

const SYSTEM = [
  "Voce narra rotas/roteiros JA CALCULADOS por um motor deterministico (sobre os",
  "municipios do Brasil). Regras invioláveis:",
  "- NUNCA altere, recalcule ou invente distancias, municipios, estados ou numeros.",
  "- Use SOMENTE os dados fornecidos no JSON; nao adicione municipios fora das listas.",
  "- Se houver o campo \"erro\" ou faltar dado, diga isso explicitamente; nao preencha lacunas.",
  "Produza um resumo claro e conciso em portugues (3 a 5 frases).",
].join(" ");

/** astar: 1 waypoint por estado em rota longa; todos se curta. */
function waypointsPorEstado(caminho: string[]): string[] {
  const out: string[] = [];
  let uf = "";
  for (const m of caminho) {
    const u = m.split("/")[1] ?? "";
    if (u !== uf) { out.push(m); uf = u; }
  }
  const ult = caminho[caminho.length - 1];
  if (out[out.length - 1] !== ult) out.push(ult);
  return out;
}

function buildAstar(rota: RotaAstar): Prompt {
  if (rota.erro === "sem_caminho" || rota.km_total === null) {
    return {
      system: SYSTEM,
      user:
        "Esta rota NAO existe (sem caminho terrestre entre origem e destino). " +
        "Explique isso brevemente, sem inventar uma rota.\n\nDados:\n" +
        JSON.stringify(rota, null, 2),
    };
  }
  const total = rota.caminho.length;
  const curta = total <= MAX_LISTA;
  const municipios_a_citar = curta ? rota.caminho : waypointsPorEstado(rota.caminho);
  const instrucao = curta
    ? "Resuma a rota citando, na ordem, TODOS os municipios de \"municipios_a_citar\"."
    : `Resuma a rota citando, na ordem, os municipios de "municipios_a_citar" (pontos de entrada em cada estado; a rota tem ${total} municipios no total). NAO liste todos os ${total}.`;
  const payload = {
    origem: rota.origem, destino: rota.destino, km_total: rota.km_total,
    total_municipios: total, estados: rota.estados, expansao: rota.expansao,
    municipios_a_citar,
  };
  const user =
    `${instrucao} Inclua distancia total (km), estados atravessados e o ganho do A* sobre o Dijkstra.` +
    `\n\nDados (fonte de verdade, nao alterar):\n` + JSON.stringify(payload, null, 2);
  return { system: SYSTEM, user };
}

function buildTsp(rota: RotaTsp): Prompt {
  if (rota.erro) {
    return {
      system: SYSTEM,
      user: `O roteiro nao pode ser calculado (erro: ${rota.erro}). Explique brevemente, sem inventar.`,
    };
  }
  const paradas = rota.paradas.map((p) => `${p.nome}/${p.uf}`);
  const payload = {
    tipo: rota.fechado ? "ciclo fechado (volta a origem)" : "rota aberta (sem retorno)",
    km_total: rota.km_total,
    km_inicial_nn: rota.km_inicial_nn,
    reducao_percent: rota.reducao_percent,
    total_municipios_no_trajeto: rota.caminho_completo.length,
    paradas_em_ordem: paradas,
  };
  const user =
    "Resuma o ROTEIRO OTIMIZADO: diga se e ciclo fechado ou rota aberta; cite as paradas " +
    "na ordem de \"paradas_em_ordem\"; informe a distancia total (km), a reducao em relacao a " +
    "rota gulosa (nearest-neighbor) e quantos municipios o trajeto completo percorre. " +
    "Use SOMENTE estes dados.\n\nDados (fonte de verdade, nao alterar):\n" +
    JSON.stringify(payload, null, 2);
  return { system: SYSTEM, user };
}

export function buildPrompt(rota: Rota): Prompt {
  return isTsp(rota) ? buildTsp(rota) : buildAstar(rota as RotaAstar);
}
