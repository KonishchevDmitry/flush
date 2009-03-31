/*************************************************************************
 ** Sample1
 ** Load sample.cfg and increment the "X" setting
 *************************************************************************/

#include <stdio.h>
#include <libconfig.h>

struct config_t cfg;

/***************************************************************************/

int main()
{
  /* Initialize the configuration */
  config_init(&cfg);

  /* Load the file */
  printf("loading [sample.cfg]..");
  if (!config_read_file(&cfg, "sample.cfg"))
    printf("failed\n");
  else
  {
    config_setting_t *setting = NULL;
    
    printf("ok\n");
    
    /* Get the "x" setting from the configuration.. */
    printf("increment \"x\"..");
    setting = config_lookup(&cfg, "x");
    if (!setting)
      printf("failed\n");
    else
    {
      long x = config_setting_get_int(setting);
      x++;
      config_setting_set_int(setting, x);
      printf("ok (x=%lu)\n", x);

      /* Save the changes */
      printf("saving [sample.cfg]..");
      config_write_file(&cfg, "sample.cfg");
      printf("ok\n");

      printf("Done!\n");
    }
  }

  /* Free the configuration */
  config_destroy(&cfg);

  return 0;
}

/***************************************************************************/
