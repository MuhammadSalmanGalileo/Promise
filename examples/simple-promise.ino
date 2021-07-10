#include <Promise.h>

Promise<int>* asyncFunc(int i){
  return new Promise<int>([i](){
    vTaskDelay(2000);
    return i;
  });
}

void scaleTask(void *pvParameters) {
  vTaskDelay(5000);
  vTaskDelete(NULL);
};

void setup() {
  Serial.begin(115200);
};
 
void loop() {
  Serial.println(ESP.getFreeHeap());
  int i = random(1, 10);
  Promise<int> promise = Promise<int>([=](){  
    vTaskDelay(2000);
    return i;
  });
  Serial.println(promise.await());
  Promise<int>* asyncFuncResult = asyncFunc(i);
  Serial.println(asyncFuncResult->await());
  delete asyncFuncResult;
};