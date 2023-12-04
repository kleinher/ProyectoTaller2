import React from 'react'
import HookMqtt from './components/Hook/'
import TemperaturesChart from './components/Hook/Grafico'

import './App.css'

function App() {

  return (
    <div className="App">
      <HookMqtt />
      <TemperaturesChart />
    </div>
  )
}

export default App
