#pragma once

namespace taowin {


class tabctrl : public syscontrol
{
public:
    // 插入一个标签
    // i: 索引，从 0 开始
    // s: 索引文本
    // p: 用户数据
    int insert_item(int i, const TCHAR* s, const void* p);

    // 获取窗口/显示区域大小
    // large: 是求大的还是小的（大的：根据显示区域大小写TAB控件窗口大小）
    // rc: 传入、传出
    void adjust_rect(bool large, RECT* rc);

protected:
    virtual void get_metas(syscontrol_metas& metas, std::map<string, string>& attrs) override;
};

}
