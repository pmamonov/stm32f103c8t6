#ifndef __BT_H__
#define __BT_H__

#define GPIO_BT_DISC	1

int bt_pair(struct iofun *rw);
int bt_auto(struct iofun *rw);

#endif /* __BT_H__ */
