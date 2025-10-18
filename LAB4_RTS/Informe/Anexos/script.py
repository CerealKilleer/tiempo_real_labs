import re
import statistics
import matplotlib
import matplotlib.pyplot as plt
import csv

ARCHIVO = "salida.txt"
UMBRAL_OFFSET = 500
EXPORTAR_CSV = True

with open(ARCHIVO, "r", encoding="utf-8") as f:
    lines = f.readlines()

patterns = {
    "speed_sensor": re.compile(r"\[speed_sensor thread\].*?([\d.]+) ms"),
    "fuel_injection": re.compile(r"\[fuel_injection thread\].*?([\d.]+) ms"),
    "position_sensor": re.compile(r"\[position_sensor\].*?([\d.]+) ms"),
    "abs_control": re.compile(r"\[abs_control thread\].*?([\d.]+) ms")
}

data = {key: [] for key in patterns}
first_seen = {key: False for key in patterns}

for line in lines:
    for key, regex in patterns.items():
        match = regex.search(line)
        if match:
            val = float(match.group(1))
            # Ignorar la primera medición (offset inicial)
            if not first_seen[key]:
                first_seen[key] = True
                continue
            # Filtrar valores grandes solo para hilos rápidos
            if key == "abs_control" or val < UMBRAL_OFFSET:
                data[key].append(val)

def stats(values, nominal=None):
    if not values:
        return None
    avg = statistics.mean(values)
    stdev = statistics.pstdev(values)
    jitter_max = max(values) - min(values)
    jitter_abs = max(abs(v - avg) for v in values)
    return {
        "n": len(values),
        "avg": avg,
        "std": stdev,
        "jitter_max": jitter_max,
        "jitter_abs": jitter_abs,
        "nominal_error": (avg - nominal) if nominal else None
    }

nominals = {
    "speed_sensor": 20,
    "fuel_injection": 80,
    "position_sensor": 200,
    "abs_control": 40
}

report = {}
for key in data:
    report[key] = stats(data[key], nominals.get(key))

print("\n Resumen de tiempos (ms):\n")
for key, r in report.items():
    if r:
        print(f"{key:17s} -> muestras={r['n']:<5d} "
              f"promedio={r['avg']:.3f}  σ={r['std']:.3f}  "
              f"jitter={r['jitter_max']:.3f}  error_prom={r['nominal_error']:.3f}")
    else:
        print(f"{key:17s} -> No se encontraron datos válidos")

if EXPORTAR_CSV:
    with open("reporte_tiempos.csv", "w", newline="") as csvfile:
        writer = csv.writer(csvfile)
        writer.writerow(["Hilo", "Muestras", "Promedio (ms)", "Desv. estándar", "Jitter máx", "Error promedio"])
        for key, r in report.items():
            if r:
                writer.writerow([
                key,
                r['n'],
                f"{r['avg']:.4f}",
                f"{r['std']:.4f}",
                f"{r['jitter_max']:.4f}",
                f"{r['nominal_error']:.8f}" if r['nominal_error'] is not None else ""
            ])

    print("\n Resultados exportados a 'reporte_tiempos.csv'")
