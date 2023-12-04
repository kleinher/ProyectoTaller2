import React, { createContext, useEffect, useState } from 'react'

import Receiver from './Receiver'
import mqtt from 'mqtt'

export const QosOption = createContext([])

const HookMqtt = () => {
  const [client, setClient] = useState(null)
  const [isSubed, setIsSub] = useState(false)
  const [payload, setPayload] = useState({})
  const [connectStatus, setConnectStatus] = useState('Connect')
  //Me conecto al broker
  useEffect(() => {
    const protocol= 'ws'
    const host= '163.10.3.73'
    const clientId= 'hernan'
    const port=8083
    const username= 'taller2g2'
    const password= 'taller2g2'
    setConnectStatus('Connecting')
    const url = `${protocol}://${host}:${port}/mqtt`
    const options = {
      clientId,
      username,
      password,
      clean: true,
      reconnectPeriod: 1000, // ms
      connectTimeout: 30 * 1000, // ms
    }
    setClient(mqtt.connect(url, options))
  },[])

  useEffect(() => {
    if (client) {
      client.on('connect', () => {
        setConnectStatus('Connected')
        console.log('connection successful')
      })

      client.on('error', (err) => {
        console.error('Connection error: ', err)
        client.end()
      })

      client.on('reconnect', () => {
        setConnectStatus('Reconnecting')
      })

      client.on('message', (topic, message) => {
        const payload = { topic, message: message.toString() }
        setPayload(payload)
      })
    }
  }, [client])

  /*Para usar en actuador.*/
  const mqttPublish = (context) => {
    if (client) {
      const { topic, qos, payload } = context
      client.publish(topic, payload, { qos }, (error) => {
        if (error) {
          console.log('Publish error: ', error)
        }
      })
    }
  }
 

  useEffect (() => {
      const subscription = {
      topic: 'Arduino/temp',
      qos: 0,
      }
    if (client) {
      const { topic, qos } = subscription

      client.subscribe(topic, { qos }, (error) => {
        if (error) {
          console.log('Subscribe to topics error', error)
          return
        }
        console.log(`Subscribe to topics: ${topic}`)
        setIsSub(true)
      })
    }
  },[client])

  return (
    <>
      <Receiver payload={payload} />
    </>
  )
}

export default HookMqtt
