import paho.mqtt.client as mqtt
import logging
import json
import mysql.connector
from mysql.connector import Error
from flask import Flask, jsonify
from flask_cors import CORS
app = Flask(__name__)
CORS(app)

# Configura el logging
logging.basicConfig(filename='./logs/app.log', level=logging.INFO)

# Configuración de las credenciales de la base de datos
config = {
    'user': 'taller2g2',
    'password': 'taller2g2',
    'host': '163.10.3.73',
    'port': 3306,
    'database': 'taller2',
    'raise_on_warnings': True
}

# Función para insertar datos en la base de datos
def insertar_temperatura(temp):
    try:
        conn = mysql.connector.connect(**config)
        cursor = conn.cursor()
        insert_query = "INSERT INTO medidas (fecha_hora, temp) VALUES (NOW(), %s)"
        cursor.execute(insert_query, (temp,))
        conn.commit()
        logging.info("Registro insertado con éxito.")
    except Error as e:
        logging.error(f"Error al insertar datos en MySQL: {e}")
    finally:
        if conn.is_connected():
            cursor.close()
            conn.close()

# Callback cuando se conecta al broker
def on_connect(client, userdata, flags, rc):
    logging.info(f"Conectado con código de resultado {rc}")
    client.subscribe("Arduino/temp")

# Callback cuando se recibe un mensaje del tópico
def on_message(client, userdata, msg):
    data = json.loads(msg.payload.decode())
    temperatura = data["temp"]
    insertar_temperatura(temperatura)
    logging.info(f"Mensaje recibido: {temperatura} en el tópico {msg.topic}")

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.username_pw_set("taller2g2", "taller2g2")
client.connect("163.10.3.73", 1883, 60)

# Endpoint para obtener las temperaturas
@app.route('/temperaturas', methods=['GET'])
def obtener_temperaturas():
    try:
        conn = mysql.connector.connect(**config)
        cursor = conn.cursor()
        cursor.execute("SELECT fecha_hora, temp FROM medidas ORDER BY fecha_hora DESC LIMIT 20")
        rows = cursor.fetchall()
        columnas = cursor.column_names
        resultados = [dict(zip(columnas, row)) for row in rows]
        resultados.reverse()
        return jsonify(resultados)
    except Error as e:
        return jsonify({"error": str(e)}), 500
    finally:
        if conn.is_connected():
            cursor.close()
            conn.close()

if __name__ == '__main__':
    client.loop_start()
    app.run(debug=True, port=5000)
