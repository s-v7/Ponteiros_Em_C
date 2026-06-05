-- =====================================================================
-- arestas_municipios_pi.sql  (database: postgres | table: public.municipios_geo)
-- Edges = neighborhood by shared border, robust against slivers
-- (typical IBGE data micro-gaps/overlaps) using a 50 m tolerance
-- in geography. Each pair is returned ONCE (a.codigo < b.codigo).
-- IMPORTANT: does not export weight. The weight (haversine) is calculated in C
-- using the SAME function as the heuristic -> single source of truth -> admissible A*.
-- Usage: psql -h localhost -U postgres -d postgres -f queries/arestas_municipios_pi.sql
-- =====================================================================

\copy (SELECT a.codigo_ibge AS origem, b.codigo_ibge AS destino FROM public.municipios_geo a JOIN public.municipios_geo b ON a.codigo_ibge < b.codigo_ibge AND ST_DWithin(a.geometry::geography, b.geometry::geography, 50) WHERE a.uf = 'PI' AND b.uf = 'PI' ORDER BY origem, destino) TO 'arestas.csv' CSV HEADER
