#include "main.h"

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	elm_init(argc, argv);
	elm_policy_set(ELM_POLICY_QUIT, ELM_POLICY_QUIT_LAST_WINDOW_CLOSED);

	create_view(&ad);

	elm_run();
	elm_shutdown();

	return ret;
}
