# FrameEasyx
一个用于编写Windows窗体程序的库，基于EasyX图形库。
EasyX图形库的官方网站:<https://easyx.cn/>
#快速开始与实例
> 配置环境

前往EasyX图形库的[官方网站](https://easyx.cn/)，下载EasyX的安装包，然后依照指示安装到你的IDE中。如果你对EasyX不太了解，建议安装EasyX的帮助文件（或访问[帮助页面](https://docs.easyx.cn/zh-cn/intro)），以便在遇到问题时阅读或在空闲时学习。
如果你使用的IDE不是Visual Studio系列，请查看[本教程](https://codebus.cn/bestans/easyx-for-mingw)

新建一个空白项目，将FrameEasyX.h头文件添加到项目中，然后新建一个cpp文件。
然后，写入以下代码：
```c++
#include "FrameEasyX.h" //包含FrameEasyx库和EasyX库（FrameEasyx库已经包含EasyX库）
Picture img_btn_box = Picture(SIZE{ 200,50 }, 255, COORD{ 50,50 }, IDentifier{ L"img_btn_box" });
Button btn_box = Button(&img_btn_box, IDentifier{ L"btn_box" });

Picture img_btn_exit = Picture(SIZE{ 200,50 }, 255, COORD{ 50,150 }, IDentifier{ L"img_btn_exit" });
Button btn_exit = Button(&img_btn_exit, IDentifier{ L"btn_exit" });

volatile char exit_flag; //程序退出标志

//消息循环线程函数
DWORD WINAPI MessageLoop(_In_ LPVOID lpParameter) {
	ExMessage msg{}; //消息循环中获取的消息
	while (true) {
		msg = getmessage(); //获取消息，如果没有消息就一直等待知道有新消息

    //处理消息
		if (btn_exit.isClinked(msg)) {
			InterlockedExchange8(&exit_flag, true); //用原子操作函数把退出标志变为true，告诉main主函数程序退出
			return 0; //结束消息处理线程
		}
		else if (btn_box.isClinked(msg)) {
			MessageBox(GetHWnd(), L"Hello,world!", L"你好，世界！", MB_OK | MB_ICONINFORMATION);//弹出一个提示框
		}
	}
}

int main() {
	initgraph(640, 480); //初始化窗口
  //初始化控件，将控件进行一些绘制

	//将按钮控件btn_box的图片填满RGB值为255,255,255的颜色(即白色)
	img_btn_box.fillimage_alpha(RGB(255,255,255));
	//往按钮控件btn_box的图片的左上角打印“你好,世界!”的文本
	img_btn_box.outtextxy_alpha(5, 5, L"你好,世界!", RGB(127, 127, 127), RGB(255, 255, 255), newfont(36, 0));
	//将按钮控件btn_box的图片填满RGB值为255,0,0的颜色(即红色)
	img_btn_exit.fillimage_alpha(RGB(255, 0, 0));
	//往按钮控件btn_box的图片的左上角打印“退出”的文本
	img_btn_box.outtextxy_alpha(5, 5, L"退出", RGB(127, 127, 127), RGB(255, 255, 255), newfont(36, 0));

	/*
	开始批量绘图，批量绘图先将绘制的操作存储到缓冲区内，然后在执行FlushBatchDraw()函数后将缓冲区内的内容一次性绘制到窗口上
	*/
	BeginBatchDraw();
  CreateThread(NULL, 10240, &MessageLoop, NULL, 0, NULL); //创建并开始消息循环的线程
	while (true) {
		//绘制控件（绘制结果先存入批量绘图的缓冲区中）
		btn_box.print();
		btn_exit.print();
		if (exit_flag == true) { //如果退出标志为真
			break; //跳出循环，程序退出
		}
		FlushBatchDraw();//通过批量绘图把控件一次性绘制到窗口上
	}
	EndBatchDraw(); //结束批量绘图
	closegraph(); //关闭窗口
	return 0;
}
```

如果一切顺利，编译后，你能够看到一个窗口，点击“你好,世界!”按钮能弹出弹窗，点击“退出”能退出程序。
