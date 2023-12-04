import React, { useEffect, useState } from 'react';
import Thermometer from 'react-thermometer-component'
const Receiver = ({ payload }) => {
  const [messages, setMessages] = useState(30)
  useEffect(() => {
    if (payload.topic) {
      const parsedMessage = JSON.parse(payload.message);
      const temperatura = parsedMessage.temp;
      if(temperatura != 'error'){
        setMessages(temperatura)
      }
      else{
        console.log('error')
      }
    }
  }, [payload])


  return (
    <>
      <h1 style={{color:'white'}}>Temperatura</h1>
        <Thermometer
          theme="dark"
          value={messages}
          max="100"
          steps="5"
          format="°C"
          size="large"
          height="400"
        />
    </>
  );
}

export default Receiver;
