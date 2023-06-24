import csv
import folium
import matplotlib.pyplot as plt
import mpld3
import matplotlib.dates as mdates

# Ruta al archivo CSV
archivo_csv = 'datos.csv'

# Listas para almacenar latitudes y longitudes
latitudes = []
longitudes = []
columna3 = []
columna4 = []
columna5 = []
columna6 = []
columna7 = []
columna8 = []
columna9 = []
columna10 = []
columna11 = []
columna12 = []

# Leer el archivo CSV y extraer los datos
with open(archivo_csv, 'r') as file:
    csv_reader = csv.reader(file)
    next(csv_reader)  # Saltar la primera fila
    for row in csv_reader:
        latitudes.append(float(row[0]))  # Convertir a flotante
        longitudes.append(float(row[1]))  # Convertir a flotante
        columna3.append(float(row[2]))  # Convertir a flotante
        columna4.append(float(row[3]))  # Convertir a flotante
        columna5.append(float(row[4]))  # Convertir a flotante
        columna6.append(float(row[5]))  # Convertir a flotante
        columna7.append(float(row[6]))  # Convertir a flotante
        columna8.append(float(row[7]))  # Convertir a flotante
        columna9.append(float(row[8]))  # Convertir a flotante
        columna10.append(float(row[9]))  # Convertir a flotante
        columna11.append(float(row[10]))  # Convertir a flotante



# Crear un mapa con ancho y alto personalizados
mapa = folium.Map(location=[latitudes[0], longitudes[0]], width='100%', height='50%')

# Agregar marcadores para cada punto
for lat, lon in zip(latitudes, longitudes):
    folium.Marker(location=[lat, lon], icon_size=(2, 10)).add_to(mapa)

# Trazar una ruta que conecte los puntos
puntos = list(zip(latitudes, longitudes))
folium.PolyLine(locations=puntos, color='red', weight=2).add_to(mapa)

# Ajustar los límites del mapa automáticamente
mapa.fit_bounds(puntos)

# Crear una figura y graficar las columnas 3, 4 y 5
fig, (ax,gyro,mag) = plt.subplots(1,3,figsize=(12, 4))
ax.plot(columna3[1:], label='Aceleración X')
ax.plot(columna4[1:], label='Aceleración Y')
ax.plot(columna5[1:], label='Aceleración Z')

# Configurar etiquetas y título de la gráfica
ax.set_xlabel('Índice')
ax.set_ylabel('Valores')
ax.set_title('Gráfica de la Aceleracion')
ax.legend()
ax.grid()

# Crear una figura y graficar las columnas 6, 7 y 8

gyro.plot(columna6[1:], label='Gyro X')
gyro.plot(columna7[1:], label='Gyro Y')
gyro.plot(columna8[1:], label='Gyro Z')

# Configurar etiquetas y título de la gráfica
gyro.set_xlabel('Índice')
gyro.set_ylabel('Valores')
gyro.set_title('Gráfica del Giroscopio')
gyro.legend()
gyro.grid()

mag.plot(columna9[1:], label='Magneto X')
mag.plot(columna10[1:], label='Magneto Y')
mag.plot(columna11[1:], label='Magneto Z')

# Configurar etiquetas y título de la gráfica
mag.set_xlabel('Índice')
mag.set_ylabel('Valores')
mag.set_title('Gráfica del Magnetometro')
mag.legend()
mag.grid()

# Crear una cadena de texto con el título centrado
titulo_html = '<h1 style="text-align: center;">Telemetría Dron</h1>'

# Agregar el título y la gráfica al archivo HTML del mapa
mapa.get_root().html.add_child(folium.Element(titulo_html))
# Convertir la gráfica de matplotlib a HTML
grafica_html = mpld3.fig_to_html(fig)

# Agregar la gráfica al archivo HTML del mapa
mapa.get_root().html.add_child(folium.Element(grafica_html))

# Guardar el mapa como un archivo HTML
mapa.save('mapa.html')
