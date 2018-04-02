#include "qtec_ebtables_basic.h"

static int lock_fd=-1;

struct ebtables_speedlimit_rule* alloc_speedlimit_rule()
{
	struct ebtables_speedlimit_rule *rule = calloc(1,sizeof(*rule));

	if(rule){
		memset(rule,0,sizeof(*rule));
		list_add_tail(&rule->list,&global_ebtables_speedlimit_rule);
		rule->enabled=true;
	}

	return rule;
}

void
qtec_ebtables_free_list(struct list_head *head)
{
	struct list_head *entry, *tmp;

	if (!head)
		return;

	list_for_each_safe(entry, tmp, head)
	{
		list_del(entry);
		free(entry);
	}
	 INIT_LIST_HEAD(head);
	//free(head);
	//head = NULL;
}


bool qtec_ebtables_lock(void)
{
	lock_fd = open(QTEC_EBTABLES_LOCKFILE, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);

	if (lock_fd < 0)
	{
		DEBUG_PRINTF("[%s] Cannot create lock file %s: %s",__func__, QTEC_EBTABLES_LOCKFILE, strerror(errno));
		return false;
	}

	if (flock(lock_fd, LOCK_EX))
	{
		DEBUG_PRINTF("[%s] Cannot acquire exclusive lock: %s", __func__, strerror(errno));
		return false;
	}

	return true;
}

void qtec_ebtables_unlock(void)
{
	if (lock_fd < 0)
		return;

	if (flock(lock_fd, LOCK_UN))
		DEBUG_PRINTF("[%s] Cannot release exclusive lock: %s", __func__, strerror(errno));

	close(lock_fd);
	unlink(QTEC_EBTABLES_LOCKFILE);

	lock_fd = -1;
}


