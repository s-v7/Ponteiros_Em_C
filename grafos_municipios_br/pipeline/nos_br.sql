-- nos_br.sql  (database where geobr was loaded | table municipios_geo_br)
-- Exports 1 node per municipality: code_muni, name, state, lat, lon.
-- code_muni is cast to integer text to avoid values like "1100015.0"
-- and to match the edge table.
-- CRS 4674 (SIRGAS 2000) already uses latitude/longitude in degrees,
-- so the Haversine formula can consume it directly.
-- Usage: psql -h localhost -U postgres -d postgres -f pipeline/nos_br.sql

\copy (SELECT code_muni::bigint::text AS code_muni, name_muni AS name, abbrev_state AS state, ST_Y(ST_PointOnSurface(geometry)) AS lat, ST_X(ST_PointOnSurface(geometry)) AS lon FROM municipios_geo_br ORDER BY name_muni) TO 'data/nos_br.csv' CSV HEADER
