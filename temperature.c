/****************************************************************************
 * apps/examples/hello/hello_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include "temperature.h"
#include "common.h"

#include <stdio.h>
#include <fcntl.h>
#include <mqueue.h>

mqd_t g_temperature_mq;

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * reflow_temperature
 ****************************************************************************/

static int temperature_task(int argc, char *argv[])
{
  int16_t temp;
  int fd0;
  int ret;
  fd0 = open("/dev/temp0", O_RDONLY);

  while (1)
  {
    printf("Channel SSP0/SPI1 Device 0: ");
    if (fd0 < 0)
    {
      /* The file could not be open,
       * probably the device is not registered
       */

      printf("Not enabled!\n");
    }
    else
    {
      ret = read(fd0, &temp, 2);
      if (ret < 0)
      {
        /* The file could not be read, probably some max31855 pin is
         * not connected to the channel.
           */
        printf("Disconnected!\n");
      }
      else
      {
        /* Print temperature value of target device */
        printf("Temperature = %d!\n", temp / 4);
        ret = mq_send(g_temperature_mq, (char *)&temp, sizeof(temp), 0);
        if (ret < 0)
        {
          printf("Failed to send message to queue: %d\n", errno);
        }
      }
    }

    /* One second sample rate */
    usleep(1000000);
  }

  return 0;
}

int create_temperature_mq(void)
{
  struct mq_attr attr;

  attr.mq_maxmsg = MQ_MAX_MSG;
  attr.mq_msgsize = MQ_MSG_SIZE;

  mq_unlink(MQ_NAME);
  g_temperature_mq = mq_open(MQ_NAME, O_RDWR | O_CREAT, 0666, &attr);
  if (g_temperature_mq == (mqd_t)-1)
  {
    printf("Failed to open message queue: %d\n", errno);
    return -1;
  }

  return 0;
}

int close_temperature_mq(void)
{
  int ret;

  ret = mq_close(g_temperature_mq);
  if (ret < 0)
  {
    printf("Failed to close message queue: %d\n", errno);
    return -1;
  }

  return 0;
}

int init_temperature(void)
{
  int ret;

  ret = task_create("temperature", 100, 2048, temperature_task, NULL);
  if (ret < 0)
  {
    printf("Failed to start temperature task: %d\n", ret);
    return ret;
  }

  return 0;
}