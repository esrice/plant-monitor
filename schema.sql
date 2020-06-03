CREATE TABLE dht_readings(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    time DATETIME DEFAULT CURRENT_TIMESTAMP,
    temperature NUMERIC,
    humidity NUMERIC,
    moisture NUMERIC,
    light NUMERIC
);
