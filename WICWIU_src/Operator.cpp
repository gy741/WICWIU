#include "Operator.h"

template class Operator<int>;
template class Operator<float>;
template class Operator<double>;

template<typename DTYPE> Operator<DTYPE>::Operator(std::string pName) {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::Operator()" << '\n';
    #endif  // __DEBUG__
    m_aaResult            = NULL;
    m_aaGradient          = NULL;
    m_apOutput            = NULL;
    m_apInput             = NULL;
    m_OutputDegree        = 0;
    m_InputDegree         = 0;
    m_currentOutputDegree = 0;
    m_currentInputDegree  = 0;
    m_name                = pName;
    m_Device              = CPU;
    m_isTensorholder      = 0;
    m_isTrainable         = 0;
    m_numOfThread         = 1;
    Alloc();
}

template<typename DTYPE> Operator<DTYPE>::Operator(Operator<DTYPE> *pInput, std::string pName) {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::Operator()" << '\n';
    #endif  // __DEBUG__
    m_aaResult            = NULL;
    m_aaGradient          = NULL;
    m_apOutput            = NULL;
    m_apInput             = NULL;
    m_OutputDegree        = 0;
    m_InputDegree         = 0;
    m_currentOutputDegree = 0;
    m_currentInputDegree  = 0;
    m_name                = pName;
    m_Device              = CPU;
    m_isTensorholder      = 0;
    m_isTrainable         = 0;
    m_numOfThread         = 1;
    Alloc(1, pInput);
}

template<typename DTYPE> Operator<DTYPE>::Operator(Operator<DTYPE> *pInput0, Operator<DTYPE> *pInput1, std::string pName) {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::Operator()" << '\n';
    #endif  // __DEBUG__
    m_aaResult            = NULL;
    m_aaGradient          = NULL;
    m_apOutput            = NULL;
    m_apInput             = NULL;
    m_OutputDegree        = 0;
    m_InputDegree         = 0;
    m_currentOutputDegree = 0;
    m_currentInputDegree  = 0;
    m_name                = pName;
    m_Device              = CPU;
    m_isTensorholder      = 0;
    m_isTrainable         = 0;
    m_numOfThread         = 1;
    Alloc(2, pInput0, pInput1);
}

template<typename DTYPE> Operator<DTYPE>::~Operator() {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::~Operator()" << '\n';
    #endif  // __DEBUG__
    this->Delete();
}

template<typename DTYPE> int Operator<DTYPE>::Alloc() {
    m_aaResult   = new Container<Tensor<DTYPE> *>();
    m_aaGradient = new Container<Tensor<DTYPE> *>();
    m_apOutput   = new Container<Operator<DTYPE> *>();
    m_apInput    = new Container<Operator<DTYPE> *>();

    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::Alloc(int numInput, ...) {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::Alloc(Tensor<DTYPE> *)" << '\n';
    #endif  // __DEBUG__
    Operator<DTYPE> *temp = NULL;

    m_aaResult   = new Container<Tensor<DTYPE> *>();
    m_aaGradient = new Container<Tensor<DTYPE> *>();
    m_apOutput   = new Container<Operator<DTYPE> *>();
    m_apInput    = new Container<Operator<DTYPE> *>();

    va_list ap;
    va_start(ap, numInput);

    for (int i = 0; i < numInput; i++) {
        temp = va_arg(ap, Operator<DTYPE> *);

        if (temp) {
            this->AddEdgebetweenOperators(temp);
        } else {
            for (int j = i - 1; j > -1; j--) {
                delete (*m_apInput)[j];
            }
            delete m_apInput;
            m_apInput = NULL;

            printf("Receive NULL pointer of Operator<DTYPE> class in %s (%s %d)\n", __FUNCTION__, __FILE__, __LINE__);
            return FALSE;
        }
    }

    va_end(ap);

    return TRUE;
}

template<typename DTYPE> void Operator<DTYPE>::Delete() {
    #if __DEBUG__
    std::cout << "Operator<DTYPE>::Delete()" << '\n';
    #endif  // __DEBUG__
    int size = 0;

    if (m_aaResult) {
        size = m_aaResult->GetSize();
        Tensor<DTYPE> **ResultContainer = m_aaResult->GetRawData();

        for (int i = 0; i < size; i++) {
            delete ResultContainer[i];
            ResultContainer[i] = NULL;
        }

        delete m_aaResult;
        m_aaResult = NULL;
    }

    if (m_aaGradient) {
        size = m_aaGradient->GetSize();
        Tensor<DTYPE> **GradientContainer = m_aaGradient->GetRawData();

        for (int i = 0; i < size; i++) {
            if ((*m_aaGradient)[i]) {
                delete GradientContainer[i];
                GradientContainer[i] = NULL;
            }
        }

        delete m_aaGradient;
        m_aaGradient = NULL;
    }

    if (m_apOutput) {
        delete m_apOutput;
        m_apOutput = NULL;
    }

    if (m_apInput) {
        delete m_apInput;
        m_apInput = NULL;
    }
}

#if __CUDNN__
template<typename DTYPE> void Operator<DTYPE>::InitializeAttributeForGPU() {}

template<typename DTYPE> void Operator<DTYPE>::SetCudnnHandle(cudnnHandle_t& pCudnnHandle) {
    m_pCudnnHandle = pCudnnHandle;
    this->InitializeAttributeForGPU();
}

void cudnnResize(int size, float *data) {
    if (data == NULL) {
        checkCudaErrors(cudaFree(data));
    }
    checkCudaErrors(cudaMalloc(&data, size * sizeof(float)));
}

#endif  // if __CUDNN__

template<typename DTYPE> void Operator<DTYPE>::SetResult(Tensor<DTYPE> *pTensor) {
    if (m_aaResult->GetSize()) {
        Tensor<DTYPE> *temp = m_aaResult->Pop();
        delete temp;
        temp = NULL;
    }

    m_aaResult->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::AddResult(Tensor<DTYPE> *pTensor) {
    m_aaResult->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::SetGradient(Tensor<DTYPE> *pTensor) {
    if (m_aaGradient->GetSize()) {
        Tensor<DTYPE> *temp = m_aaGradient->Pop();
        delete temp;
        temp = NULL;
    }

    m_aaGradient->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::AddGradient(Tensor<DTYPE> *pTensor) {
    m_aaGradient->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::SetDelta(Tensor<DTYPE> *pTensor) {
    if (m_aaGradient->GetSize()) {
        Tensor<DTYPE> *temp = m_aaGradient->Pop();
        delete temp;
        temp = NULL;
    }

    m_aaGradient->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::AddDelta(Tensor<DTYPE> *pTensor) {
    m_aaGradient->Push(pTensor);
}

template<typename DTYPE> void Operator<DTYPE>::IncreaseCurrentOutputDegree() {
    m_currentOutputDegree++;
}

template<typename DTYPE> void Operator<DTYPE>::IncreaseCurrentInputDegree() {
    m_currentInputDegree++;
}

#if __CUDNN__
template<typename DTYPE> cudnnHandle_t& Operator<DTYPE>::GetCudnnHandle() {
    return m_pCudnnHandle;
}

#endif  // if __CUDNN__

template<typename DTYPE> Tensor<DTYPE> *Operator<DTYPE>::GetResult() const {
    return (*m_aaResult)[0];
}

template<typename DTYPE> Container<Tensor<DTYPE> *> *Operator<DTYPE>::GetResultContainer() {
    return m_aaResult;
}

template<typename DTYPE> Tensor<DTYPE> *Operator<DTYPE>::GetGradient() const {
    return (*m_aaGradient)[0];
}

template<typename DTYPE> Container<Tensor<DTYPE> *> *Operator<DTYPE>::GetGradientContainer() {
    return m_aaGradient;
}

template<typename DTYPE> Tensor<DTYPE> *Operator<DTYPE>::GetDelta() const {
    return (*m_aaGradient)[0];
    // return (*m_aaDelta)[0];
}

template<typename DTYPE> Container<Tensor<DTYPE> *> *Operator<DTYPE>::GetDeltaContainer() {
    return m_aaGradient;
    // return m_aaDelta;
}

template<typename DTYPE> Operator<DTYPE> **Operator<DTYPE>::GetOutput() {
    return m_apOutput->GetRawData();
}

template<typename DTYPE> Container<Operator<DTYPE> *> *Operator<DTYPE>::GetOutputContainer() {
    return m_apOutput;
}

template<typename DTYPE> Operator<DTYPE> **Operator<DTYPE>::GetInput() {
    return m_apInput->GetRawData();
}

template<typename DTYPE> Container<Operator<DTYPE> *> *Operator<DTYPE>::GetInputContainer() {
    return m_apInput;
}

template<typename DTYPE> int Operator<DTYPE>::GetOutputDegree() const {
    return m_OutputDegree;
}

template<typename DTYPE> int Operator<DTYPE>::GetInputDegree() const {
    return m_InputDegree;
}

template<typename DTYPE> int Operator<DTYPE>::GetCurrentOutputDegree() const {
    return m_currentOutputDegree;
}

template<typename DTYPE> int Operator<DTYPE>::GetCurrentInputDegree() const {
    return m_currentInputDegree;
}

template<typename DTYPE> std::string Operator<DTYPE>::GetName() const {
    return m_name;
}

// Add Graph Edge
template<typename DTYPE> int Operator<DTYPE>::_AddInputEdge(Operator<DTYPE> *pInput) {
    try {
        m_apInput->Push(pInput);
    } catch (...) {
        printf("Failed to allcate memory in %s (%s %d)\n", __FUNCTION__, __FILE__, __LINE__);
        return FALSE;
    }
    m_InputDegree++;

    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::_AddOutputEdge(Operator<DTYPE> *pOutput) {
    try {
        m_apOutput->Push(pOutput);
    } catch (...) {
        printf("Failed to allcate memory in %s (%s %d)\n", __FUNCTION__, __FILE__, __LINE__);
        return FALSE;
    }
    m_OutputDegree++;

    return TRUE;
}

template<typename DTYPE> void Operator<DTYPE>::AddEdgebetweenOperators(Operator<DTYPE> *pInput) {
    this->_AddInputEdge(pInput);
    pInput->_AddOutputEdge(this);
}

template<typename DTYPE> int Operator<DTYPE>::ForwardPropagate(int pTime, int pThreadNum) {
    #if __DEBUG__
    std::cout << "thread number : " << pThreadNum << '\n';
    std::cout << "number of thread : " << this->GetNumOfThread() << '\n';
    #endif  // __DEBUG__
    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::BackPropagate(int pTime, int pThreadNum) {
    #if __DEBUG__
    std::cout << "thread number : " << pThreadNum << '\n';
    std::cout << "number of thread : " << this->GetNumOfThread() << '\n';
    #endif  // __DEBUG__
    return TRUE;
}

#if __CUDNN__
template<typename DTYPE> int Operator<DTYPE>::ForwardPropagateOnGPU(int pTime) {
    # if __DEBUG__
    std::cout << "Operator<DTYPE>::ForwardPropagateOnGPU(int)" << '\n';
    std::cout << this->GetName() << '\n';
    # endif // __DEBUG__
    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::BackPropagateOnGPU(int pTime) {
    # if __DEBUG__
    std::cout << "Operator<DTYPE>::BackPropagateOnGPU(int)" << '\n';
    std::cout << this->GetName() << '\n';
    # endif // __DEBUG__
    return TRUE;
}

#endif  // __CUDNN__

template<typename DTYPE> void Operator<DTYPE>::SetModeTraining() {
    // std::cout << "Operator<DTYPE>::SetModeTraining()" << '\n';
}

template<typename DTYPE> void Operator<DTYPE>::SetModeAccumulating() {
    // std::cout << "Operator<DTYPE>::SetModeAccumulating()" << '\n';
}

template<typename DTYPE> void Operator<DTYPE>::SetModeInferencing() {
    // std::cout << "Operator<DTYPE>::SetModeInferencing()" << '\n';
}

template<typename DTYPE> void Operator<DTYPE>::SetDeviceCPU() {
    m_Device = CPU;

#if __CUDNN__
    this->SetResultCPU();
    this->SetGradientCPU();
#endif  // __CUDNN__
}

template<typename DTYPE> void Operator<DTYPE>::SetDeviceCPU(int pNumOfThread) {
    m_Device      = CPU;
    m_numOfThread = pNumOfThread;

#if __CUDNN__
    this->SetResultCPU();
    this->SetGradientCPU();
#endif  // __CUDNN__
}

#if __CUDNN__
template<typename DTYPE> int Operator<DTYPE>::SetResultCPU() {
    // Tensorholder의 경우는 하면 안된다.
    int size = m_aaResult->GetSize();

    for (int i = 0; i < size; i++) {
        (*m_aaResult)[i]->MemcpyDeviceToHost();
    }

    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::SetGradientCPU() {
    int size = m_aaGradient->GetSize();

    for (int i = 0; i < size; i++) {
        (*m_aaGradient)[i]->MemcpyDeviceToHost();
    }

    return TRUE;
}

template<typename DTYPE> void Operator<DTYPE>::SetDeviceGPU() {
    m_Device = GPU;
    this->SetResultGPU();
    this->SetGradientGPU();
}

template<typename DTYPE> int Operator<DTYPE>::SetResultGPU() {
    // Tensorholder의 경우는 하면 안된다.
    int size = m_aaResult->GetSize();

    for (int i = 0; i < size; i++) {
        (*m_aaResult)[i]->MemcpyHostToDevice();
    }

    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::SetGradientGPU() {
    int size = m_aaGradient->GetSize();

    for (int i = 0; i < size; i++) {
        (*m_aaGradient)[i]->MemcpyHostToDevice();
    }

    return TRUE;
}

#endif  // __CUDNN__

template<typename DTYPE> int Operator<DTYPE>::ResetResult() {
    // Tensorholder의 경우는 하면 안된다.
    int size = m_aaResult->GetSize();

    if (m_Device == CPU) {
        for (int i = 0; i < size; i++) {
            (*m_aaResult)[i]->Reset();
        }
    }

#if __CUDNN__
    else if (m_Device == GPU) {
        for (int i = 0; i < size; i++) {
            (*m_aaResult)[i]->Reset(this->GetCudnnHandle());
        }
    }
#endif // if __CUDNN__

    else return FALSE;

    return TRUE;
}

template<typename DTYPE> int Operator<DTYPE>::ResetGradient() {
    int size = m_aaGradient->GetSize();

    if (m_Device == CPU) {
        for (int i = 0; i < size; i++) {
            (*m_aaGradient)[i]->Reset();
        }
    }

#if __CUDNN__
    else if (m_Device == GPU) {
        for (int i = 0; i < size; i++) {
            (*m_aaGradient)[i]->Reset(this->GetCudnnHandle());
        }
    }
#endif // if __CUDNN__

    else return FALSE;

    return TRUE;
}

template<typename DTYPE> void Operator<DTYPE>::PrintInformation() {
    std::cout << this->GetName() << " : ";
    std::cout << this->GetResult()->GetShape() << '\n';
}

// int main(int argc, char const *argv[]) {
// Operator<int> *temp1 = new Operator<int>("temp1");
// Operator<int> *temp2 = new Operator<int>(temp1, "temp2");
// Operator<int> *temp3 = new Operator<int>(temp1, temp2, "temp3");
//
// std::cout << temp3->GetInput()[0]->GetName() << '\n';
// std::cout << temp3->GetInput()[1]->GetName() << '\n';
// std::cout << temp1->GetOutput()[0]->GetName() << '\n';
//
// delete temp1;
// delete temp2;
// delete temp3;
//
// return 0;
// }