#pragma once

namespace taowin {


class tabctrl : public syscontrol
{
public:
    // ����һ����ǩ
    // i: �������� 0 ��ʼ
    // s: �����ı�
    // p: �û�����
    int insert_item(int i, const TCHAR* s, const void* p);

    // ��ȡ����/��ʾ�����С
    // large: �����Ļ���С�ģ���ģ�������ʾ�����СдTAB�ؼ����ڴ�С��
    // rc: ���롢����
    void adjust_rect(bool large, RECT* rc);

protected:
    virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
};

}
