import paho.mqtt.client as mqtt
import logging
import json
import mysql.connector
from mysql.connector import Error
# Configura el logging
logging.basicConfig(filename='/home/taller2g2/logs/app.log', level=logging.INFO)

# Configuración de las credenciales de la base de datos
config = {
    'user': 'taller2g2',
    'password': 'taller2g2',
    'host': '163.10.3.73',
    'port': 3306,
    'database': 'taller2',
    'raise_on_warnings': True
}

# Callback cuando se conecta al broker
def on_connect(client, userdata, flags, rc):
    logging.info(f"Conectado con código de resultado {rc}")
    # Nos suscribimos al tópico (en este caso "test/topic")
    client.subscribe("Arduino/temp")


# Callback cuando se recibe un mensaje del tópico
def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    try:
        conn = mysql.connector.connect(**config)
        # Crear un nuevo cursor
        cursor = conn.cursor()

        # Comando SQL para insertar un nuevo registro
        insert_query = """
        INSERT INTO medidas (fecha_hora, temp) VALUES (NOW(), %s)
        """

        # Datos que quieres insertar
        # Asegúrate de que la fecha y hora estén en el formato correcto que tu base de datos espera
        # Por ejemplo, podría ser un string en formato 'AAAA-MM-DD HH:MM:SS' para fecha y hora
        datos = (data["temp"],)

        # Ejecutar el comando SQL
        cursor.execute(insert_query, datos)

        # Hacer commit de la transacción
        conn.commit()

        print("Registro insertado con éxito.")
    except Error as e:
        print(f"Error al insertar datos en MySQL: {e}")
    finally:
    # Cerrar el cursor y la conexión
        if conn.is_connected():
            cursor.close()
            conn.close()
            print("La conexión a MySQL ha sido cerrada.")

    logging.info(f"Mensaje recibido: {data['temp']} en el tópico {msg.topic}")
    


logging.info("test")
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

logging.info("conectando")
client.username_pw_set("taller2g2", "taller2g2")
# Establecer la conexión
try:
    conn = mysql.connector.connect(**config)
    print("Conexión exitosa a la base de datos.")
except mysql.connector.Error as e:
    print(f"Error al conectar a MySQL: {e}")
# Conexión al broker

client.connect("163.10.3.73", 1883, 60)
logging.info("Conectado")

# Loop para mantener el cliente activo y escuchando mensajes
client.loop_forever()

