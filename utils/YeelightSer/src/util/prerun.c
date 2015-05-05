/*
 * prerun.c
 *
 *  Created on: 2015年5月5日
 *      Author: yeso
 */

#include "prerun.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "logger.h"

//int8_t flag = 0;	/*0：nothing 1:退出*/
#define LOCKMODE 0644	/* 创建掩码 */
static const char *LOCK_FILE = "/var/run/YeelightSer.pid";	/* 锁文件 */
extern char* DEMO_NAME;



/*
 * 初始化守护进程
 */
void init_daemon(int exitflag){
	umask(0);
	switch(fork()){
		case 0:
			setsid();
			break;
		case -1:
			LOG_ERROR("%s start error！",DEMO_NAME);
			exit(EXIT_FAILURE);
			break;
		default:
			exit(EXIT_SUCCESS);
			break;
	}


	check_running(exitflag);	//注意,这个不能放在fork之前

	LOG_TRACE("%s is started!",DEMO_NAME);

//	chdir("/");
//	int null_fd = open ("/dev/null", O_RDWR , 0);
//	if (null_fd != -1)
//	{
//	 dup2 (null_fd, STDIN_FILENO);
//	 dup2 (null_fd, STDOUT_FILENO);
//	 dup2 (null_fd, STDERR_FILENO);
//
//	 if (null_fd > 2)
//		close (null_fd);
//	}
}


/*
 * 检查程序是否已经运行，确保只启动一个进程
 */
void check_running(int exitflag){
	struct flock fl;
	int32_t lockfd = -1;	/* 锁文件描述符 */

	lockfd = open (LOCK_FILE, O_RDWR|O_CREAT, LOCKMODE);
	if(lockfd < 0){
		LOG_TRACE("打开锁文件失败，请检查是否为root权限！");
		exit(EXIT_FAILURE);
	}

	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	fl.l_type = F_WRLCK;

	if (fcntl(lockfd, F_GETLK, &fl) < 0) {
		LOG_TRACE("获取文件锁失败，请检查是否为root权限！");
		exit(EXIT_FAILURE);
	}

	if(exitflag){	//程序退出
		if ( fl.l_type != F_UNLCK ) {
			if ( kill ( fl.l_pid, SIGKILL ) == -1) {
				LOG_ERROR("结束 %s 进程失败!",DEMO_NAME);
			}else{
				LOG_INFO("%s 进程(PID=%d)已经退出.",DEMO_NAME , fl.l_pid);
			}
		}else{
			LOG_INFO("当前没有 %s 进程正在运行",DEMO_NAME);
		}
		exit(EXIT_SUCCESS);
	}else if(fl.l_type != F_UNLCK){	//该程序已经有进程在运行
		LOG_INFO("%s 已经运行!(PID=%d)",DEMO_NAME , fl.l_pid);
		exit(EXIT_FAILURE);
	}
	fl.l_type = F_WRLCK ;
	fl.l_pid = getpid () ;
	if (fcntl(lockfd, F_SETLKW, &fl) < 0) {
		perror("%s 加锁失败!" , DEMO_NAME);
		exit ( EXIT_FAILURE ) ;
	}
}
