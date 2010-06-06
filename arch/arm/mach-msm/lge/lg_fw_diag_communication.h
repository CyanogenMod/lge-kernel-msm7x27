/*
 *   LG_FW_AUDIO_TESTMODE
 *
 *   kiwone creates this file for audio test mode, and the use of another function to send framework.
*/
#ifndef	LG_FW_DIAG_H
#define	LG_FW_DIAG_H

struct diagcmd_dev {
	const char	*name;
	struct device	*dev;
	int		index;
	int		state;
};

struct diagcmd_platform_data {
	const char *name;
};

extern int diagcmd_dev_register(struct diagcmd_dev *sdev);
extern void diagcmd_dev_unregister(struct diagcmd_dev *sdev);

static inline int diagcmd_get_state(struct diagcmd_dev *sdev)
{
	return sdev->state;
}

extern void update_diagcmd_state(struct diagcmd_dev *sdev, char *cmd, int state);
extern struct diagcmd_dev *diagcmd_get_dev(void);

#endif	// LG_FW_DIAG_H

