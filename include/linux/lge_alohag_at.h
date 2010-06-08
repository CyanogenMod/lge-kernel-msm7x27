/* LGE_CHANGE_S [kimeh@lge.com] 2009-04-15, for debugging */
struct atcmd_dev {
//	const char	*name;
	char	*name;
	struct device	*dev;
	int		index;
	int		state;
};

struct atcmd_platform_data {
	const char *name;
};

extern int atcmd_dev_register(struct atcmd_dev *sdev);
extern void atcmd_dev_unregister(struct atcmd_dev *sdev);

static inline int atcmd_get_state(struct atcmd_dev *sdev)
{
	return sdev->state;
}

extern void update_atcmd_state(struct atcmd_dev *sdev, char *cmd, int state);
extern struct atcmd_dev *atcmd_get_dev(void);
/* LGE_CHANGE_E [kimeh@lge.com] 2009-04-15, for debugging */


/* LGE_CHANGE_S [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
struct ats_mtc_key_log_type{
	unsigned char log_id;
	unsigned short log_len;
	unsigned int x_hold;
	unsigned int y_code;
	unsigned char action;
};

enum ats_mtc_key_log_id_type{
	ATS_MTC_KEY_LOG_ID_KEY = 1,
	ATS_MTC_KEY_LOG_ID_TOUCH = 2,
	ATS_MTC_KEY_LOG_ID_MAX,
};

/* LGE_CHANGE_E [jihoon.lee@lge.com] 2010-02-03, LG_FW_ATS_ETA_MTC_KEY_LOGGING */
