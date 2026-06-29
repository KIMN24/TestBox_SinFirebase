#pragma once

class ProcessInterface {
public:
    virtual ~ProcessInterface() = default;
    virtual void execute() = 0;
    virtual void setTiempoAbrir(int tiempo) = 0;
    virtual void setTiempoCerrar(int tiempo) = 0;
    virtual void setTiempoLazo(int tiempo) = 0;
    virtual int getTiempoAbrir() = 0;
    virtual int getTiempoCerrar() = 0;
    virtual int getTiempoLazo() = 0;
    virtual String getModeName() = 0;
};
