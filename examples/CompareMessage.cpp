#include <iostream>
// ʹ�þ�̬�����Ҫ������ mirai.h ǰ���������
#define MIRAICPP_STATICLIB
#include <mirai.h>

int main()
{
	using namespace std;
	using namespace Cyan;
	system("chcp 65001");

	MessageChain mc1 = MessageChain().Plain("##").At(123456_qq).Face(14).Plain("Hello");
	MessageChain mc2 = MessageChain().Plain("##").At(1234569_qq).Face(14).Plain("Hello");

	if (mc1 == mc2)
		cout << "mc1 == mc2" << endl;
	else
		cout << "mc1 != mc2" << endl;

	if (mc1 != mc2)
		cout << "mc1 != mc2" << endl;
	else
		cout << "mc1 == mc2" << endl;

	return 0;
}