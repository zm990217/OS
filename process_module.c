#include<stdio.h>
#include<stdlib.h>

//起始标记与结束标记
long offsetStart = -1, offsetEnd = -1;

int main()
{
	//加载内核模块
	system("insmod process_module.ko");

	FILE* fp; //文件指针，用于读取内核日志信息
	char ch;  //信息读取单元
	char Buffer[100];  //所读取的信息存放处
	//打开内核日志文件
	if ((fp = fopen("/var/log/kern.log", "r")) == NULL) {
		printf("File open error!\n");
		exit(0);
	}
	//定向文件指针到文件开头
	fseek(fp, 0L, SEEK_SET);
	//搜索标识符
	while (!feof(fp)) {
		//如果读取到所加载的内核模块打印信息的起始标识符
		if ((ch = fgetc(fp)) == '#' && (ch = fgetc(fp)) == '#' && (ch = fgetc(fp)) == '#') {
			//确定读取信息的起始位置
			offsetStart = ftell(fp) - 3;
		}
	}
	rewind(fp);

	//搜索标识符
	while (!feof(fp)) {
		//如果读取到所加载的内核模块打印信息的结束标识符
		if ((ch = fgetc(fp)) == '!' && (ch = fgetc(fp)) == '!' && (ch = fgetc(fp)) == '!') {
			offsetEnd = ftell(fp) - 3;
		}
	}
	rewind(fp);

	//将文件指针定向到标记的起始位置
	fseek(fp, offsetStart, SEEK_SET);

	//读取文件信息直至结束标记处
	while (ftell(fp) < offsetEnd)
	{
		//获取信息
		fgets(Buffer, 100, fp);
		char* pstr;
		pstr = Buffer;
		int i= 0;
		while(*pstr!='#' && (*pstr!='!' && *(pstr+1)!='!') && (*pstr != '-')){
			pstr++;
		}
		//打印信息
		printf("%s", pstr);
	}

	//关闭文件
	if (fclose(fp))
	{
		printf("Can not close the file!\n");
		exit(0);
	}

	//卸载内核模块
	system("rmmod process_module.ko");
	return 0;
}
