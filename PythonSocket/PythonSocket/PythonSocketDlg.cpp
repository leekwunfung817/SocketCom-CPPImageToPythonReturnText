
// PythonSocketDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PythonSocket.h"
#include "PythonSocketDlg.h"
#include "afxdialogex.h"






#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CPythonSocketDlg dialog



CPythonSocketDlg::CPythonSocketDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPythonSocketDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPythonSocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_PATH_NAME, m_edit_path_name);
	DDX_Control(pDX, IDC_EDIT_RECEIVE, m_edit_receive);
	DDX_Control(pDX, IDC_EDIT_SERVER_PORT, m_edit_server_port);
	DDX_Control(pDX, IDC_EDIT_SERVER_URL, m_edit_server_url);
	DDX_Control(pDX, IDC_EDIT_TIMEOUT, m_edit_timeout);
	DDX_Control(pDX, IDC_CHECK_CHECKSUM, m_check_checksum);
}

BEGIN_MESSAGE_MAP(CPythonSocketDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_B_OPEN, &CPythonSocketDlg::OnBnClickedBOpen)
	ON_BN_CLICKED(IDC_B_SEND, &CPythonSocketDlg::OnBnClickedBSend)
	ON_BN_CLICKED(IDOK, &CPythonSocketDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CPythonSocketDlg message handlers

BOOL CPythonSocketDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon



	m_edit_server_url.SetWindowTextW(_T("127.0.0.1"));
	m_edit_server_port.SetWindowTextW(_T("65432"));
	m_edit_timeout.SetWindowTextW(_T("3"));


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CPythonSocketDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CPythonSocketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CPythonSocketDlg::OnBnClickedBOpen()
{
	CFileDialog dlgOpen(TRUE, _T("*.*"), _T(""), NULL, _T(""));//"*.dat|*.dat|All Files|*.*|"

	if (dlgOpen.DoModal() == IDOK)
	{
		CString strFilePath = dlgOpen.GetPathName();
		m_edit_path_name.SetWindowTextW(strFilePath);

	}
	else
	{
		return;
	}


}

#include <Winsock2.h>
#pragma comment(lib,"ws2_32.lib")


//����У���룬���Ȳ�����У���뱾��
unsigned char checksum(unsigned char *str, int len)
{
	int i;
	unsigned char mod = 0;
	for (i = 0; i<len; i++)
	{
		mod += *(str + i);
	}
	return -mod;
}

CString send(CString strUrl, CString strPort, CString strFileName, CString strTimeOut, int check)
{
	int i = 0;
	char send_head[1] = { 'A' };
	char pic_name[40] = { 0 };
	unsigned char checksum = 0;

	CString strReceive;
	WSADATA wsaData;
	SOCKET sockClient;//�ͻ���Socket
	SOCKADDR_IN addrServer;//����˵�ַ
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	//�½��ͻ���socket
	sockClient = socket(AF_INET, SOCK_STREAM, 0);

	char chServer[256] = { 0 };
	char chClientId[256] = { 0 };

	WideCharToMultiByte(CP_ACP, 0, strUrl, -1, chServer, sizeof(chServer) - 1, NULL, NULL);


	CString strFile;
	int iPos = 0;
	iPos = strFileName.ReverseFind('\\');
	if (iPos < 0)  strFileName.ReverseFind('/');
	if (iPos >= 0)
	{
		strFile = strFileName.Right(strFileName.GetLength() - iPos - 1);
		WideCharToMultiByte(CP_ACP, 0, strFile, -1, pic_name, sizeof(pic_name) - 1, NULL, NULL);
	}


	int timeout = _wtoi(strTimeOut);
	if (timeout < 1) timeout = 1;

	//����Ҫ���ӵķ���˵�ַ
	addrServer.sin_addr.s_addr = inet_addr((const char*)chServer);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = htons(_wtoi(strPort));//���Ӷ˿�


	//��ʱ����, ����/���ճ�ʱ
	struct timeval timeoutRecv = { 0 };
	timeoutRecv.tv_sec = timeout * 1000;
	//���ͳ�ʱ
	setsockopt(sockClient, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeoutRecv, sizeof(timeoutRecv));
	//���ճ�ʱ
	setsockopt(sockClient, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeoutRecv, sizeof(timeoutRecv));

	int iResult = 0;
	//���ӵ������
	iResult = connect(sockClient, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));

	if (iResult == NO_ERROR)
	{
		int rc = 0;
		CFile file;
		if (file.Open(strFileName, CFile::modeRead))
		{
			UINT filesize = 0;
			filesize = file.GetLength();

			file.SeekToBegin();

			char sendbuf[1024 * 16];//ÿ�η�16KB
			size_t nread = 0;
			char *ptr = 0;
			size_t count = 0;

			if (check)
			{

				sendbuf[0] = send_head[0];//0 - ���
				nread = 1;
				*(UINT *)(&sendbuf[1]) = filesize;	//byte 1,2,3,4 - length
				nread += 4;
				memcpy(&sendbuf[5], pic_name, sizeof(pic_name));	//40 byte filename
				nread += sizeof(pic_name);

				for (i = 0; i < nread; i++) checksum += sendbuf[i];//����checksum
				rc = send(sockClient, sendbuf, nread, 0);//��������ͷ���ļ�����

				if (rc < 0)	//<0 SOCKET_ERROR��������	==0���������, ����ʵ��δ��������,�����ٷ�
				{
					//������
				}
			}

			do {
				nread = file.Read(sendbuf, sizeof(sendbuf));
				if (nread <= 0)
				{
					//�����ļ�β��
					//rc = SFTP_ERR_TRUE;
					break;
				}

				for (i = 0; i<nread; i++) checksum += sendbuf[i];//����checksum

				ptr = sendbuf;
				int zero_cnt = 0;	//��������0����Ĵ���, ��ֹ��ѭ��
				do {

					rc = send(sockClient, ptr, nread, 0);//����ʵ�ʷ��͵����ݸ���,�������0����ʾ����,��δ��������
					if (rc < 0)	//<0 SOCKET_ERROR��������	==0���������, ����ʵ��δ��������,�����ٷ�
					{
						break;
					}
					if (rc == 0)
					{
						zero_cnt++;	//ͳ�����������Ĵ���
						if (zero_cnt >= 3) break;//����3��, ��ʾʧ��
					}
					else
					{
						zero_cnt = 0;
					}

					ptr += rc;
					count += nread;
					nread -= rc;
				} while (nread);

				//	if ( m_isBreak == true ) break;  //�û���ֹ�ϴ�

			} while (rc > 0);

			if (check)
			{
				sendbuf[0] = -checksum;//checksum Ϊ����
				rc = send(sockClient, sendbuf, 1, 0);//��������ͷ���ļ�����
			}

			//������ɺ�, �رշ���, python���յ��ر���Ϣ���˳�����ѭ��
			shutdown(sockClient, SD_SEND);


			//���շ�����Ϣ
			char recvBuf[32] = { 0 };
			int rs = 0;
			int recv_len = 0;

			do
			{
				memset(recvBuf, 0, sizeof(recvBuf));
				recv_len = recv(sockClient, recvBuf, 31, 0);

				if (recv_len < 0)
				{					
					break;
				}
				else if (recv_len == 0)
				{
					// �����ʾ�Զ˵�socket�������ر�.
				}

				if (recv_len != 31)//��������Ԥ�ڣ��Ѷ���
				{
					rs = 0;
				}
				else//�պ���ȣ�����δ����
				{
					rs = 1;// ��Ҫ�ٴζ�ȡ
				}

				strReceive += CString(recvBuf);
			} while (rs);

		}
		file.Close();

	}


	closesocket(sockClient);
	WSACleanup();

	return strReceive;
}



void CPythonSocketDlg::OnBnClickedBSend()
{
	CString strFileName;
	CString strUrl;
	CString strPort;
	CString strTimeOut;
	m_edit_path_name.GetWindowTextW(strFileName);
	m_edit_server_url.GetWindowTextW(strUrl);
	m_edit_server_port.GetWindowTextW(strPort);

	m_edit_timeout.GetWindowTextW(strTimeOut);
	CString strReceive;
	strReceive = send(strUrl, strPort, strFileName, strTimeOut, m_check_checksum.GetCheck());

	m_edit_receive.SetWindowTextW(strReceive);
}


void CPythonSocketDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here
	CDialogEx::OnOK();
}
