#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

void setup() {
    Serial.begin(115200);
    while (!Serial); // Aguarda a inicialização do monitor serial

    Wire.begin(21, 22); // Define os pinos SDA e SCL no ESP32

    if (!mpu.begin()) {
        Serial.println("MPU6050 não encontrado! Verifique a conexão.");
        while (1);
    }

    Serial.println("MPU6050 encontrado!");

    // Configuração do sensor
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
}

void loop() {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    Serial.print("Acelerômetro (m/s^2): X="); Serial.print(a.acceleration.x);
    Serial.print(", Y="); Serial.print(a.acceleration.y);
    Serial.print(", Z="); Serial.println(a.acceleration.z);

    Serial.print("Giroscópio (°/s): X="); Serial.print(g.gyro.x);
    Serial.print(", Y="); Serial.print(g.gyro.y);
    Serial.print(", Z="); Serial.println(g.gyro.z);

    Serial.print("Temperatura: "); Serial.print(temp.temperature);
    Serial.println("°C");

    Serial.println("---------------------");
    delay(500); // Aguarda 500ms antes da próxima leitura
}
