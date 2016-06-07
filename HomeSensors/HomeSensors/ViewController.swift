//
//  ViewController.swift
//  HomeSensors
//
//  Created by Tomas Krejci on 6/7/16.
//  Copyright © 2016 Tomas Krejci. All rights reserved.
//

import UIKit
import Moscapsule

class ViewController: UIViewController {
    
    var mqttConfig: MQTTConfig! = nil
    var mqttClient: MQTTClient? = nil

    @IBOutlet weak var startButton: UIButton!
    @IBOutlet weak var temperatureLabel: UILabel!
    @IBOutlet weak var humidityLabel: UILabel!
    @IBOutlet weak var airQualityLabel: UILabel!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        
        mqttConfig = MQTTConfig(clientId: UIDevice.currentDevice().identifierForVendor!.UUIDString, host: "10.0.1.10", port: 1883, keepAlive: 60)
        
        mqttConfig.onMessageCallback = { mqttMessage in
            let isTimestampOnly = mqttMessage.payloadString?.containsString(":")
            if isTimestampOnly == nil || isTimestampOnly! {
                print("Skipping .. ")
                return
            }
            
            var parts = mqttMessage.payloadString?.componentsSeparatedByString(",")
            if parts != nil && parts!.count == 2 {
                print("Whoooo \(parts![1])")
                dispatch_async(dispatch_get_main_queue(), {
                    switch mqttMessage.topic {
                    case "temperature":
                        self.temperatureLabel.text = parts![1]
                    case "humidity":
                        self.humidityLabel.text = parts![1]
                    case "air_quality":
                        self.airQualityLabel.text = parts![1]
                    default:
                        break
                    }
                })
            }
            
        }
        
        connectToMqttServer()
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Dispose of any resources that can be recreated.
    }
    
    @IBAction func startButton(sender: UIButton) {
        print("Button pressed")
        if mqttClient == nil {
            connectToMqttServer()
            startButton.setTitle("Stop", forState: .Normal)
        } else {
            disconnectFromMqttServer()
            startButton.setTitle("Start", forState: .Normal)
        }
    }
    
    func connectToMqttServer() {
        mqttClient = MQTT.newConnection(mqttConfig)
        
        mqttClient!.subscribe("temperature", qos: 0)
        mqttClient!.subscribe("air_quality", qos: 0)
        mqttClient!.subscribe("humidity", qos: 0)
    }
    
    func disconnectFromMqttServer() {
        mqttClient!.disconnect()
        mqttClient = nil
    }
}

