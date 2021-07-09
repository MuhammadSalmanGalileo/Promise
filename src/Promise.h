#include "Arduino.h"
#include <functional>

template <typename T>
class Promise {
    private:
        SemaphoreHandle_t promiseSemaphore = NULL;
        TaskHandle_t asyncTaskHandle = NULL;
        void waitForLock();
        void unlockLock();
        T result;
    public:
        std::function<T()> asyncFunc;
        Promise();
        ~Promise(){
            vSemaphoreDelete(promiseSemaphore);
            if(asyncTaskHandle != NULL)
                vTaskDelete(asyncTaskHandle);
        };
        Promise(std::function<T()> asyncFunc, int stackSize = 3000, String taskName = "Resolver Task");
        T await();
        void resolvePromise(T result);
        void cleanTaskDelete() {
            TaskHandle_t taskHandle = asyncTaskHandle;
            asyncTaskHandle = NULL;
            vTaskDelete(taskHandle);
        };
        friend void promiseResolver(void *pvParameters);
};

template <typename T>
void promiseResolver(void *pvParameters) {
    Promise<T>* promise = static_cast<Promise<T>*>(pvParameters);
    T result = promise->asyncFunc();
    promise->resolvePromise(result);
    promise->cleanTaskDelete();
    vTaskDelete(NULL);
}

template <typename T>
Promise<T>::Promise(){
    promiseSemaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(promiseSemaphore);
    xSemaphoreTake(promiseSemaphore, portMAX_DELAY);
};

template <typename T>
Promise<T>::Promise(std::function<T()> asyncFunc, int stackSize, String taskName)
    :Promise() {
    this->asyncFunc = asyncFunc;
    xTaskCreate(promiseResolver<T>,
        "test",
        stackSize,
        static_cast<void*>(this),
        1, 
        &asyncTaskHandle);
};

template <typename T>
void Promise<T>::resolvePromise(T result){
    this->result = result;
    unlockLock();
};

template <typename T>
T Promise<T>::await(){
    waitForLock();
    return result;
};

template <typename T>
void Promise<T>::waitForLock(){
    xSemaphoreTake(promiseSemaphore, portMAX_DELAY);
};

template <typename T>
void Promise<T>::unlockLock(){
    xSemaphoreGive(promiseSemaphore);
};