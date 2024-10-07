int bits[8] = {2, 3, 12, 30, 18, 22, 23, 24};
int pins[8] = {1, 0, 24, 26, 14, 17, 16, 22};
int32_t virtualRegister;

void setup() {
  for (int idx = 0; idx < 8; idx++){
    pinMode(pins[idx], OUTPUT);
  }
}

void loop() {
  virtualRegister = GPIO6_DR;
  for (int idx = 0; idx < 8; idx++){
    virtualRegister = (virtualRegister & ~(1 << bits[idx])) | (LOW << bits[idx]);
  }
  GPIO6_DR = virtualRegister;
  delay(5000);
  for (int idx = 0; idx < 8; idx++){
    virtualRegister = (virtualRegister & ~(1 << bits[idx])) | (HIGH << bits[idx]);
  }
  GPIO6_DR = virtualRegister;
  delay(5000);
}
// GPIO6_DR = (GPIO6_DR & ~(1 << bits[idx].registerIdx)) | (bits[idx].state << bits[idx].registerIdx);