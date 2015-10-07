#include <iostream>
#include <process.h>
#include <string>
#include <memory>
#include <functional>
#include <fstream>

#include "lw_laywin.h"
#include <commctrl.h>

class qq_filerecv_monitor
{
public:
	struct qq_file_info
	{
		enum class state_e{
			creation,
			renaming,
			deleting,
		};

		qq_file_info(state_e type_)
			: type(type_)
		{}

		state_e type;
	};

	struct qq_file_info_creation : public qq_file_info
	{
		qq_file_info_creation()
		: qq_file_info(qq_file_info::state_e::creation)
		{
		}

		std::string tmpname;
		std::string sender;
	};

	struct qq_file_info_renaming : public qq_file_info
	{
		qq_file_info_renaming()
		: qq_file_info(qq_file_info::state_e::renaming)
		{}

		std::string oldname;
		std::string newname;
	};

	struct qq_file_info_deleting : public qq_file_info
	{
		qq_file_info_deleting()
		: qq_file_info(qq_file_info::state_e::deleting)
		{}

		std::string filename;
	};

public:
	qq_filerecv_monitor()
		: _hdir(nullptr)
		, _heventexit(nullptr)
		, _hthrd(nullptr)
		, _pcb(nullptr)
	{

	}
	~qq_filerecv_monitor()
	{

	}

	typedef std::function<void(const qq_file_info* file)> CB;

	void start(const std::string& path, CB pcb)
	{
		_path = path;
		_pcb = pcb;

		HANDLE hDir = CreateFile(path.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
			NULL);
		if(hDir == INVALID_HANDLE_VALUE)
			throw "无法读取此目录!";

		_hdir = hDir;

		_heventexit = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
		_hthrd = (HANDLE)::_beginthreadex(nullptr, 0, __thread_proc, this, 0, nullptr);

		if(!_heventexit || !_hthrd)
			throw "无法启动线程!";
	}

	void stop()
	{
		::SetEvent(_heventexit);
	}

private:
	static unsigned int __stdcall __thread_proc(void* pv)
	{
		auto _this = static_cast<qq_filerecv_monitor*>(pv);
		return _this->thread_read_changes();
	}

	void handle_buffer(unsigned char* buf, int size)
	{
		auto la_next_record = [](FILE_NOTIFY_INFORMATION*& p){
			if(p->NextEntryOffset)
				*(int*)&p += p->NextEntryOffset;
			else
				p = NULL;
		};

		auto la_file_name = [](FILE_NOTIFY_INFORMATION* info)->std::string{
			wchar_t wname[MAX_PATH];
			int cch = info->FileNameLength / sizeof(info->FileName[0]);
			wcsncpy(&wname[0], &info->FileName[0], cch);
			wname[cch] = 0;

			char aname[MAX_PATH] = {0};
			WideCharToMultiByte(CP_ACP, 0, wname, wcslen(wname), aname, MAX_PATH, NULL, NULL);
			return aname;
		};

		auto la_sender_name = [](){
			std::string sender;

			HWND hwin = GetForegroundWindow();
			if(hwin != nullptr){
				char win[257];
				win[GetClassName(hwin, win, sizeof(win))] = '\0';
				if(strcmp(win, "TXGuiFoundation") == 0){
					win[GetWindowText(hwin, win, sizeof(win))] = '\0';

					auto& name = win;
					int len = strlen(name);
					if(len > 6 && strcmp(name + len - 6, "个会话") == 0){
						char* p = name + len - 6;
						if(p != name){
							for(--p; p >= name && *p >= '0' && *p <= '9';){
								--p;
							}

							++p;
							if(p > name + 2){
								p -= 2;
								if(strncmp(p, "等", 2) == 0){
									*p = '\0';
								}
							}
						}
					}
					sender = win;
				}
			}
			return sender;
		};

		FILE_NOTIFY_INFORMATION* info = (FILE_NOTIFY_INFORMATION*)buf;
		while(info){
			if(info->Action == FILE_ACTION_ADDED){
				qq_file_info_creation qfic;
				qfic.tmpname = la_file_name(info);
				qfic.sender = la_sender_name();
				_pcb(&qfic);
			}
			else if(info->Action == FILE_ACTION_REMOVED){
				qq_file_info_deleting qfid;
				qfid.filename = la_file_name(info);
				_pcb(&qfid);
			}
			else if(info->Action == FILE_ACTION_RENAMED_OLD_NAME){
				qq_file_info_renaming qfir;
				qfir.oldname = la_file_name(info);
				la_next_record(info);
				if(info){
					qfir.newname = la_file_name(info);
					_pcb(&qfir);
				}
			}

			la_next_record(info);
		}
	}

	unsigned int thread_read_changes()
	{
		const int buf_len = 1 << 20;
		std::unique_ptr<unsigned char> buf(new unsigned char[buf_len]);

		class overlapped : public OVERLAPPED
		{
		public:
			overlapped()
			{
				Internal = 0;
				InternalHigh = 0;
				Offset = 0;
				OffsetHigh = 0;
				Pointer = nullptr;
			
				hEvent = ::CreateEvent(nullptr, FALSE, FALSE, nullptr);
				
			}
			~overlapped()
			{
				CloseHandle(hEvent);
			}
		};

		for(;;){
			DWORD dwBytesReturned=0;
			overlapped ov;
			BOOL b = ::ReadDirectoryChangesW(_hdir, buf.get(), buf_len, FALSE,
				FILE_NOTIFY_CHANGE_FILE_NAME,
				&dwBytesReturned,
				&ov, nullptr
				);
			if(b == FALSE){
				throw "ReadDirectoryChangesW() != TRUE";
			}

			HANDLE handles[2] = {_heventexit, ov.hEvent};
			switch(::WaitForMultipleObjects(_countof(handles), &handles[0], FALSE, INFINITE))
			{
			case WAIT_FAILED:
				throw "WaitForMultipleObjects() failed!";
			case WAIT_OBJECT_0+0:
				return 0;
			case WAIT_OBJECT_0+1:
			{
				if(GetOverlappedResult(_hdir, &ov, &dwBytesReturned, FALSE) && dwBytesReturned){
					handle_buffer(buf.get(), int(dwBytesReturned));
				}
				break;
			}
			}
		}

		return 0;
	}
private:
	std::string _path;
	HANDLE _hdir;
	HANDLE _heventexit;
	HANDLE _hthrd;
	CB _pcb;
};

class TW : public laywin::window_creator
{
public:
	TW()
		: _this_param(nullptr)
	{}

	void set_home(const laywin::string& home){
		_home = home;
	}

private:
	enum class file_state{
		tmp,
		rcvd,
		moved,
	};

	struct param_struct{
		file_state state;
		std::string name;
		std::string ext;
		std::string sender;
	};

private:
	qq_filerecv_monitor _filemon;

protected:
	void add_file(const std::string& fname, const char* state, const std::string& sender, LPARAM param=0)
	{
		auto i = _li_rcvd->insert_item(fname.c_str(), param);
		_li_rcvd->set_item(state, i, 1);
		_li_rcvd->set_item(sender.c_str(), i, 2);
	}

	void split_fname_and_ext(const std::string& filename, std::string* name, std::string* ext)
	{
		auto pos = filename.rfind('.');
		if(pos != std::string::npos){
			name->assign(filename.c_str(), pos);
			ext->assign(&filename[pos]);
		}
		else{
			name->assign(filename);
			ext->assign("");
		}
	}

protected:
	virtual LPCTSTR get_skin_json() const
	{
		LPCTSTR json =
#include "res/main.json"
			;
		return json;
	}

	virtual LRESULT handle_message(UINT umsg, WPARAM wparam, LPARAM lparam, bool& handled) override
	{
		switch(umsg)
		{
		case WM_CREATE:
		{
			_li_rcvd = (laywin::listview*)_layout->find(_T("rcvd_files"));
			_li_moved = (laywin::listview*)_layout->find(_T("moved_files"));

			_li_rcvd->insert_column(_T("文件名"), 170, 0);
			_li_rcvd->insert_column(_T("状态"), 50, 1);
			_li_rcvd->insert_column(_T("发送人"), 90, 2);
			_li_moved->insert_column(_T("文件名"), 210, 0);
			_li_moved->insert_column(_T("文件夹"), 100, 1);

			try{
				::SetCurrentDirectory(_home.c_str());
				_filemon.start(_home, [&](const qq_filerecv_monitor::qq_file_info* info){
					switch(info->type)
					{
					case qq_filerecv_monitor::qq_file_info::state_e::creation:
					{
						auto c = reinterpret_cast<const qq_filerecv_monitor::qq_file_info_creation*>(info);
						auto p = new param_struct;
						split_fname_and_ext(c->tmpname, &p->name, &p->ext);
						if(p->ext == ".tmp"){
							p->state = file_state::tmp;
							p->sender = c->sender;
							add_file(c->tmpname, "新文件", c->sender, LPARAM(p));
						}
						else{
							delete p;
						}
						break;
					}
					case qq_filerecv_monitor::qq_file_info::state_e::deleting:
					{
						auto c = reinterpret_cast<const qq_filerecv_monitor::qq_file_info_deleting*>(info);
						
						std::string name, ext;
						split_fname_and_ext(c->filename, &name, &ext);
						for(int c = _li_rcvd->size() - 1; c >= 0; --c){
							auto p = reinterpret_cast<param_struct*>(_li_rcvd->get_param(c, 0));
							if(!p) continue;
							if(p->state != file_state::moved && p->name == name && p->ext == ext){
								delete p;
								_li_rcvd->delete_item(c);

								if(_this_param == p){
									update_info(-1, nullptr);
								}
							}
						}

						break;
					}
					case qq_filerecv_monitor::qq_file_info::state_e::renaming:
						auto c = reinterpret_cast<const qq_filerecv_monitor::qq_file_info_renaming*>(info);
						for(int i = _li_rcvd->size() - 1; i >= 0; --i){
							auto p = reinterpret_cast<param_struct*>(_li_rcvd->get_param(i, 0));
							if(!p) continue;
							if(p->state==file_state::tmp && c->oldname == p->name+p->ext){
								p->state = file_state::rcvd;
								split_fname_and_ext(c->newname, &p->name, &p->ext);
								_li_rcvd->set_item(c->newname.c_str(), i, 0);
								_li_rcvd->set_item("已完成", i, 1);

								::FlashWindow(_hwnd, TRUE);
								break;
							}
						}
						break;
					}
				});
			}
			catch(const char* e){
				MessageBox(_hwnd, e, nullptr, MB_ICONERROR);
			}

			center();
			handled = true;
			return 0;
		}
		default:
			break;
		}
		return 0;
	}

	virtual LRESULT on_notify_ctrl(HWND hwnd, laywin::control* pc, int code, NMHDR* hdr) override
	{
		if(pc == _li_rcvd){
			if(code == NM_CLICK){
				auto nmclick = reinterpret_cast<NMITEMACTIVATE*>(hdr);
				if(nmclick->iItem != -1){
					auto p = (param_struct*)_li_rcvd->get_param(nmclick->iItem, 0);
					if(p->state == file_state::tmp){
						update_info(-1, nullptr);
					}
					else if(p->state == file_state::rcvd){
						update_info(nmclick->iItem, p);
					}
				}
				else{
					update_info(-1, nullptr);
				}
				return 0;
			}
			else if(code == NM_DBLCLK){
				auto nmclick = reinterpret_cast<NMITEMACTIVATE*>(hdr);
				if(nmclick->iItem != -1){
					auto p = (param_struct*)_li_rcvd->get_param(nmclick->iItem, 0);
					if(p->state == file_state::rcvd){
						::ShellExecute(_hwnd, "open", (p->name + p->ext).c_str(), NULL, NULL, SW_SHOWNORMAL);
					}
					else if(p->state == file_state::tmp){
						::MessageBox(_hwnd, _T("文件还未接收完全!"), NULL, MB_ICONEXCLAMATION);
					}
					return 0;
				}
			}
		}
		else if(pc == _li_moved){
			if(code == NM_DBLCLK){
				auto nmclick = reinterpret_cast<NMITEMACTIVATE*>(hdr);
				if(nmclick->iItem != -1){
					auto p = (param_struct*)_li_moved->get_param(nmclick->iItem, 0);
					if(p->state == file_state::moved){
						::ShellExecute(_hwnd, "open", (p->sender + "\\" + p->name + p->ext).c_str(), NULL, NULL, SW_SHOWNORMAL);
						return 0;
					}
				}
			}
		}
		else if(pc->name() == _T("btn_ok")){
			if(code == BN_CLICKED){
				if(!_this_param) return 0;

				auto edit_filename = static_cast<laywin::edit*>(_layout->find("edit_filename"));
				auto edit_foldername = static_cast<laywin::edit*>(_layout->find("edit_foldername"));
				auto filename = edit_filename->get_window_text();
				auto foldername = edit_foldername->get_window_text();

				// 移动文件会产生文件删除事件, 避免重复删除item
				_this_param->state = file_state::moved; 

				if(move_file(filename, foldername)){
					//MessageBox(_hwnd, _T("移动成功!"), "", MB_ICONINFORMATION);

					auto np = _this_param;

					_li_rcvd->delete_item(_this_i);
					_this_param = nullptr;
					_this_i = -1;

					edit_filename->set_window_text("");
					edit_foldername->set_window_text("");

					np->name = filename;
					np->ext = "";
					np->sender = foldername;
					auto idx = _li_moved->insert_item(filename.c_str(), LPARAM(np));
					_li_moved->set_item(foldername.c_str(), idx, 1);
				}
				else{
					MessageBox(_hwnd, _T("移动失败!"), "", MB_ICONERROR);
				}
			}
		}

		return __super::on_notify_ctrl(hwnd, pc, code, hdr);
	}

	void update_info(int i, param_struct* p)
	{
		_this_i = i;
		_this_param = p;

		auto edit_filename = (laywin::edit*)_layout->find("edit_filename");
		auto edit_foldername = (laywin::edit*)_layout->find("edit_foldername");

		if(p){
			edit_filename->set_window_text((p->name + p->ext).c_str());
			edit_foldername->set_window_text(p->sender.c_str());
		}
		else{
			edit_filename->set_window_text("");
			edit_foldername->set_window_text("");
		}
	}

	bool move_file(const laywin::string& fname, const laywin::string& folder){
		if(!CreateDirectory(folder.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS){
			return false;
		}

		laywin::string full = folder + "\\" + fname;

		return !!MoveFile(fname.c_str(), full.c_str());
	}

private:
	laywin::listview* _li_rcvd;
	laywin::listview* _li_moved;

	param_struct* _this_param;
	int _this_i;

	laywin::string _home;
};

laywin::string read_qq_folder()
{
	TCHAR file[MAX_PATH] = {0};
	GetModuleFileName(NULL, file, _countof(file));
	_tcscpy(_tcsrchr(file, _T('\\')) + 1, _T("movefile.txt"));

	laywin::string home;
	std::ifstream ifs(file);
	if(ifs.is_open()){
		std::getline(ifs, home);
		ifs.close();
	}
	return home;
}

#ifdef _DEBUG
int main()
#else
int __stdcall WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#endif
{

	laywin::string home = read_qq_folder();
	if(home.size() == 0){
		LPCTSTR help = _T(R"(没有正确识别到QQ文件接收目录!

请在程序目录下建立文件 `movefile.txt', 并在第一行写上
QQ文件接收目录, 没有多余的空格, 没有多余的引号.
)");

		MessageBox(NULL, help, NULL, MB_ICONINFORMATION);
		return 1;
	}

	try{
		TW tw1;
		tw1.set_home(home);
		tw1.create();
		tw1.show();

		laywin::window_manager::message_loop();
	}
	catch(LPCTSTR e){
		std::wcout << e << std::endl;
	}
	
	return 0;
}
