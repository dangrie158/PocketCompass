#ifndef __QMC5883L_H__
#define __QMC5883L_H__

#include <stdint.h>
#include "Wire.h"
#include <math.h>

/* The default I2C address of this chip */
#define QMC5883L_ADDR 0x0D

/* Register numbers */
#define QMC5883L_X_LSB 0
#define QMC5883L_X_MSB 1
#define QMC5883L_Y_LSB 2
#define QMC5883L_Y_MSB 3
#define QMC5883L_Z_LSB 4
#define QMC5883L_Z_MSB 5
#define QMC5883L_STATUS 6
#define QMC5883L_TEMP_LSB 7
#define QMC5883L_TEMP_MSB 8
#define QMC5883L_CONFIG 9
#define QMC5883L_CONFIG2 10
#define QMC5883L_RESET 11
#define QMC5883L_RESERVED 12
#define QMC5883L_CHIP_ID 13

/* Bit values for the STATUS register */
#define QMC5883L_STATUS_DRDY 1
#define QMC5883L_STATUS_OVL 2
#define QMC5883L_STATUS_DOR 4

/* Oversampling values for the CONFIG register */
#define QMC5883L_CONFIG_OS512 0b00000000
#define QMC5883L_CONFIG_OS256 0b01000000
#define QMC5883L_CONFIG_OS128 0b10000000
#define QMC5883L_CONFIG_OS64 0b11000000

/* Range values for the CONFIG register */
#define QMC5883L_CONFIG_2GAUSS 0b00000000
#define QMC5883L_CONFIG_8GAUSS 0b00010000

/* Rate values for the CONFIG register */
#define QMC5883L_CONFIG_10HZ 0b00000000
#define QMC5883L_CONFIG_50HZ 0b00000100
#define QMC5883L_CONFIG_100HZ 0b00001000
#define QMC5883L_CONFIG_200HZ 0b00001100

/* Mode values for the CONFIG register */
#define QMC5883L_CONFIG_STANDBY 0b00000000
#define QMC5883L_CONFIG_CONT 0b00000001

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif

class QMC5883L
{
public:
    void reconfig()
    {
        QMC5883L::writeRegister(this->addr, QMC5883L_CONFIG, oversampling | range | rate | mode);
    }

    void reset()
    {
        QMC5883L::writeRegister(this->addr, QMC5883L_RESET, 0x01);
        reconfig();
    }

    void setOversampling(int x)
    {
        switch (x)
        {
        case 512:
            oversampling = QMC5883L_CONFIG_OS512;
            break;
        case 256:
            oversampling = QMC5883L_CONFIG_OS256;
            break;
        case 128:
            oversampling = QMC5883L_CONFIG_OS128;
            break;
        case 64:
            oversampling = QMC5883L_CONFIG_OS64;
            break;
        }
        reconfig();
    }

    void setRange(int x)
    {
        switch (x)
        {
        case 2:
            range = QMC5883L_CONFIG_2GAUSS;
            break;
        case 8:
            range = QMC5883L_CONFIG_8GAUSS;
            break;
        }
        reconfig();
    }

    void setSamplingRate(int x)
    {
        switch (x)
        {
        case 10:
            rate = QMC5883L_CONFIG_10HZ;
            break;
        case 50:
            rate = QMC5883L_CONFIG_50HZ;
            break;
        case 100:
            rate = QMC5883L_CONFIG_100HZ;
            break;
        case 200:
            rate = QMC5883L_CONFIG_200HZ;
            break;
        }
        reconfig();
    }

    void init()
    {
        /* This assumes the wire library has been initialized. */
        addr = QMC5883L_ADDR;
        oversampling = QMC5883L_CONFIG_OS512;
        range = QMC5883L_CONFIG_2GAUSS;
        rate = QMC5883L_CONFIG_50HZ;
        mode = QMC5883L_CONFIG_CONT;
        reset();
    }

    int ready()
    {
        QMC5883L::initRead(this->addr, QMC5883L_STATUS, 1);
        uint8_t status = Wire::read();
        return status & QMC5883L_STATUS_DRDY;
    }

    int readRaw(int16_t *x, int16_t *y, int16_t *z, int16_t *t)
    {
        while (!ready())
        {
        }

        QMC5883L::initRead(this->addr, QMC5883L_X_LSB, 6);

        *x = Wire::read() | (Wire::read() << 8);
        *y = Wire::read() | (Wire::read() << 8);
        *z = Wire::read() | (Wire::read() << 8);

        Wire::stop();

        return 1;
    }

    void resetCalibration()
    {
        xhigh = yhigh = 0;
        xlow = ylow = 0;
    }

    int readHeading()
    {
        int16_t x, y, z, t;

        if (!readRaw(&x, &y, &z, &t))
            return 0;

        /* Update the observed boundaries of the measurements */

        if (x < xlow)
            xlow = x;
        if (x > xhigh)
            xhigh = x;
        if (y < ylow)
            ylow = y;
        if (y > yhigh)
            yhigh = y;

        /* Bail out if not enough data is available. */

        if (xlow == xhigh || ylow == yhigh)
            return 0;

        /* Recenter the measurement by subtracting the average */

        x -= (xhigh + xlow) / 2;
        y -= (yhigh + ylow) / 2;

        /* Rescale the measurement to the range observed. */

        float fx = (float)x / (xhigh - xlow);
        float fy = (float)y / (yhigh - ylow);

        int heading = 180.0 * atan2(fy, fx) / M_PI;
        if (heading <= 0)
            heading += 360;

        return heading;
    }

private:
    static void writeRegister(uint8_t addr, uint8_t reg, uint8_t value)
    {
        Wire::start(addr, 0);
        Wire::write(reg);
        Wire::write(value);
        Wire::stop();
    }

    static void initRead(uint8_t addr, uint8_t reg, uint8_t count)
    {
        Wire::start(addr, 0);
        Wire::write(reg);
        Wire::restart(addr, count);
    }

    int16_t xhigh, xlow;
    int16_t yhigh, ylow;
    uint8_t addr;
    uint8_t mode;
    uint8_t rate;
    uint8_t range;
    uint8_t oversampling;
};

#endif
