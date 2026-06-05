-- =====================================================================
-- nos_municipios_pi.sql  (database: postgres | table: public.municipios_geo)
-- Exports one node per municipality in PI: codigo_ibge, name, lat, lon.
-- SRID 4326 (WGS84) -> ST_Y = latitude, ST_X = longitude.
-- ST_PointOnSurface ensures a point INSIDE the polygon (≠ centroid,
-- which may fall outside in a concave municipality).
-- Usage: psql -h localhost -U postgres -d postgres -f queries/nos_municipios_pi.sql
-- =====================================================================
\copy (SELECT codigo_ibge, nome, ST_Y(ST_PointOnSurface(geometry)) AS lat, ST_X(ST_PointOnSurface(geometry)) AS lon FROM public.municipios_geo WHERE uf = 'PI' ORDER BY nome) TO 'nos.csv' CSV HEADER
