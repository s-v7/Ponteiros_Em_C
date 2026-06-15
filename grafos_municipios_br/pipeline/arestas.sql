-- arestas_br.sql  (database where geobr was loaded | table municipios_geo_br)
-- Edges = shared borders using ST_Intersects.
-- The GiST index is used through the bounding-box pre-filter,
-- making it fast at national scale.
-- Each pair is exported only once.
-- Does NOT export weight: Haversine is calculated in C,
-- using the same metric as the heuristic.
-- Usage: psql -h localhost -U postgres -d postgres -f pipeline/arestas_br.sql

\copy (SELECT a.code_muni::bigint::text AS source, b.code_muni::bigint::text AS target FROM municipios_geo_br a JOIN municipios_geo_br b ON a.code_muni < b.code_muni AND ST_Intersects(a.geometry, b.geometry) ORDER BY source, target) TO 'data/arestas_br.csv' CSV HEADER
