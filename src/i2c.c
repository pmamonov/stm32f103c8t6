#include <i2c.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

static struct i2c_cfg i2c_list[] = {
	{
		.i2c = I2C1,
		.gpio = GPIOB,
		.gpio_pins = GPIO_Pin_6 | GPIO_Pin_7,
		.rcc_gpio = &RCC->APB2ENR,
		.rcc_gpio_en = RCC_APB2Periph_GPIOB,
		.rcc_afio = &RCC->APB2ENR,
		.rcc_afio_en = RCC_APB2Periph_AFIO,
		.rcc_i2c = &RCC->APB1ENR,
		.rcc_i2c_en = RCC_APB1Periph_I2C1,
	},
	{
		.i2c = I2C2,
		.gpio = GPIOB,
		.gpio_pins = GPIO_Pin_10 | GPIO_Pin_11,
		.rcc_gpio = &RCC->APB2ENR,
		.rcc_gpio_en = RCC_APB2Periph_GPIOB,
		.rcc_afio = &RCC->APB2ENR,
		.rcc_afio_en = RCC_APB2Periph_AFIO,
		.rcc_i2c = &RCC->APB1ENR,
		.rcc_i2c_en = RCC_APB1Periph_I2C2,
	},
};

static struct i2c_cfg *i2c_get_cfg(unsigned i)
{
	if (i >= ARRAY_SIZE(i2c_list))
		return NULL;

	return &i2c_list[i];
}

static int i2c_wait_ev(unsigned i, uint32_t ev)
{
	struct i2c_cfg *cfg = i2c_get_cfg(i);
	int t;

	if (!cfg)
		return -I2C_ERR_INVAL;

	for (t = I2C_TIMEOUT; t; t--) {
		if (SUCCESS == I2C_CheckEvent(cfg->i2c, ev))
			break;
		vTaskDelay(1);
	}
	if (!t)
		return -I2C_ERR_TIMEOUT;

	return 0;
}

int i2c_init(unsigned i, unsigned bitrate)
{
	I2C_InitTypeDef i2c;
	GPIO_InitTypeDef i2c_gpio;
	struct i2c_cfg *cfg = i2c_get_cfg(i);

	if (!cfg)
		return -I2C_ERR_INVAL;

	*cfg->rcc_gpio |= cfg->rcc_gpio_en;
	*cfg->rcc_afio |= cfg->rcc_afio_en;
	*cfg->rcc_i2c |= cfg->rcc_i2c_en;

	I2C_SoftwareResetCmd(cfg->i2c, ENABLE);
	vTaskDelay(1);
	I2C_SoftwareResetCmd(cfg->i2c, DISABLE);

	I2C_StructInit(&i2c);
	i2c.I2C_ClockSpeed = bitrate;
	i2c.I2C_Mode = I2C_Mode_I2C;
	i2c.I2C_DutyCycle = I2C_DutyCycle_2;
	i2c.I2C_Ack = I2C_Ack_Enable;
	i2c.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_Init(cfg->i2c, &i2c);

	i2c_gpio.GPIO_Pin = cfg->gpio_pins;
	i2c_gpio.GPIO_Mode = GPIO_Mode_AF_OD;
	i2c_gpio.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(cfg->gpio, &i2c_gpio);

	I2C_Cmd(cfg->i2c, ENABLE);

	return 0;
}

int i2c_start(unsigned i, uint8_t dir, uint8_t addr)
{
	struct i2c_cfg *cfg = i2c_get_cfg(i);
	uint32_t mode = dir == I2C_TX ?
				I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED:
				I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED;
	int t;

	if (!cfg)
		return -I2C_ERR_INVAL;

	for (t = I2C_TIMEOUT; t; t--) {
		if (RESET == I2C_GetFlagStatus(cfg->i2c, I2C_FLAG_BUSY))
			break;
		vTaskDelay(1);
	}
	if (!t)
		return -I2C_ERR_BUSY;

	I2C_GenerateSTART(cfg->i2c, ENABLE);
	if (i2c_wait_ev(i, I2C_EVENT_MASTER_MODE_SELECT))
		return -I2C_ERR_MASTER;

	I2C_Send7bitAddress(cfg->i2c, addr << 1, dir);
	if (i2c_wait_ev(i, mode))
		return -I2C_ERR_MODE;

	return 0;
}

int i2c_stop(unsigned i)
{
	struct i2c_cfg *cfg = i2c_get_cfg(i);

	if (!cfg)
		return -I2C_ERR_INVAL;

	I2C_GenerateSTOP(cfg->i2c, ENABLE);

	return 0;
};

int i2c_xmit(unsigned i, uint8_t addr, uint8_t *data, unsigned len)
{
	struct i2c_cfg *cfg = i2c_get_cfg(i);
	int rc, n, t;

	if (!cfg)
		return -I2C_ERR_INVAL;

	if (!len)
		return 0;

	rc = i2c_start(i, I2C_TX, addr);
	if (rc)
		return rc;

	for (n = 0; n < len; n++) {
		I2C_SendData(cfg->i2c, data[n]);
		if (i2c_wait_ev(i, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
			return -I2C_ERR_XMIT;
	}

	rc = i2c_stop(i);
	if (rc)
		return rc;

	return n;
}

int i2c_recv(unsigned i, uint8_t addr, uint8_t *data, unsigned len)
{
	struct i2c_cfg *cfg = i2c_get_cfg(i);
	int rc, n, t;

	if (!cfg)
		return -I2C_ERR_INVAL;

	if (!len)
		return 0;

	rc = i2c_start(i, I2C_RX, addr);
	if (rc)
		return rc;

	for (n = 0; n < len; n++) {
		if (i2c_wait_ev(i, I2C_EVENT_MASTER_BYTE_RECEIVED))
			return -I2C_ERR_RECV;
		data[n] = I2C_ReceiveData(cfg->i2c);
	}

	rc = i2c_stop(i);
	if (rc)
		return rc;

	return n;
}
