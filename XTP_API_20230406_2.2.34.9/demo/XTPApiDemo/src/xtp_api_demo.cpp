// testTradeApi.cpp : 定义控制台应用程序的入口点。
//

#include "xtp_trader_api.h"
#include <string>
#include <cstring>
#include <map>
#include <iostream>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif // _WIN32

//#include "FileUtils.h"
#include "xtp_quote_api.h"
#include "quote_spi.h"

XTP::API::TraderApi* pUserApi;
bool is_connected_ = false;
std::string trade_server_ip;
int trade_server_port;
std::map<uint64_t,uint64_t> map_session;
uint64_t* session_arrary = NULL;
//FileUtils* fileUtils = NULL;
XTPOrderInsertInfo* orderList = NULL;
std::string quote_server_ip;
int quote_server_port;
std::string quote_username;
std::string quote_password;
XTP_PROTOCOL_TYPE quote_protocol = XTP_PROTOCOL_UDP;

int main()
{

	//读取交易配置
	trade_server_port = 6001;
	int client_id = 1;
	int account_count = 1;
#ifdef _WIN32
	std::string filepath = fileUtils->stdStringForKey("path");
#else
	std::string filepath = "./";
#endif // _WIN32
	bool ping_pong_test = false;//demo是否开启乒乓测试，默认是不开启的，如果需要开启，请在配置文件中修改，此参数仅在此demo中使用，仅涉及demo回调函数中逻辑，不是必需的

	//读取行情配置
	quote_server_ip = "119.3.103.38";
	quote_server_port = 6002;
	quote_username = "15015202";
	quote_password = "H2uw5SN9";
	quote_protocol = XTP_PROTOCOL_TCP;
	int32_t quote_buffer_size = 256;
	// 订阅的股票
	std::vector<std::string> instrument_vector = {"000977", "002230"};

	//读取心跳超时配置
	int32_t heat_beat_interval = 15;

	std::cout << "inital CreatQuoteApi begin!" << std::endl;
	//初始化行情api
	XTP::API::QuoteApi* pQuoteApi = XTP::API::QuoteApi::CreateQuoteApi(client_id, filepath.c_str(), XTP_LOG_LEVEL_DEBUG);//log日志级别可以调整
	if (!pQuoteApi)
	{
		std::cout << "Failed to create quote api, please check the input parameters." << std::endl;
		return 0;
	}
	MyQuoteSpi* pQuoteSpi = new MyQuoteSpi();
	if (!pQuoteSpi)
	{
		std::cout << "Failed to create quote spi, please check the input parameters." << std::endl;
		return 0;
	}
	std::cout << "RegisterSpi begin!" << std::endl;
	pQuoteApi->RegisterSpi(pQuoteSpi);

	//设定行情服务器超时时间，单位为秒
	pQuoteApi->SetHeartBeatInterval(heat_beat_interval); //此为1.1.16新增接口
	//设定行情本地缓存大小，单位为MB
	pQuoteApi->SetUDPBufferSize(quote_buffer_size);//此为1.1.16新增接口

	std::cout << "login begin!" << std::endl;
	int loginResult_quote = -1;
	//登录行情服务器,自1.1.16开始，行情服务器支持UDP连接，推荐使用UDP
	loginResult_quote = pQuoteApi->Login(quote_server_ip.c_str(), quote_server_port, quote_username.c_str(), quote_password.c_str(), quote_protocol); 
	if (loginResult_quote == 0)
	{
		std::cout << "login success!" << std::endl;
		//登录行情服务器成功后，订阅行情
		int instrument_count = instrument_vector.size();
		std::cout << "instrument_count.size()=" << instrument_count << std::endl;
		int quote_exchange = 2;

		//从配置文件中读取需要订阅的股票
		char* *allInstruments = new char*[instrument_count];
		for (int i = 0; i < instrument_count; i++) {
			std::cout << "instrument_count=" << i << std::endl;
			allInstruments[i] = new char[XTP_TICKER_LEN];
			std::string instrument = instrument_vector[i];
			strncpy(allInstruments[i], instrument.c_str(), XTP_TICKER_LEN - 1);
			allInstruments[i][XTP_TICKER_LEN - 1] = '\0';
			//strcpy_s(allInstruments[i], XTP_TICKER_LEN, instrument.c_str());
		}
		std::cout << "subscribeMarketData begin!" << "quote_exchang=" << quote_exchange << ", allinstrument=" << allInstruments[0] << std::endl;
		//开始订阅,注意公网测试环境仅支持TCP方式，如果使用UDP方式会没有行情数据，实盘大多数使用UDP连接
		pQuoteApi->SubscribeMarketData(allInstruments, instrument_count, (XTP_EXCHANGE_TYPE)quote_exchange);
		std::cout << "subscribeMarketData success!" << std::endl;
		//释放
		for (int i = 0; i < instrument_count; i++) {
			delete[] allInstruments[i];
			allInstruments[i] = NULL;
		}

		delete[] allInstruments;
		allInstruments = NULL;
	}
	else
	{
		//登录失败，获取失败原因
		XTPRI* error_info = pQuoteApi->GetApiLastError();
		std::cout << "Login to server error, " << error_info->error_id << " : " << error_info->error_msg << std::endl;
	
	}

	std::cout << "login after" << std::endl;
	if (account_count > 0)
	{
			
		std::cout << "account_count>0" << std::endl;
		//针对多用户的情况
		orderList = new XTPOrderInsertInfo[account_count];
		memset(orderList, 0, sizeof(XTPOrderInsertInfo)*account_count);
	}
	
	std::cout << "CreateTraderApi!!!" << std::endl;

		//主线程循环，防止进程退出
		while (true)
		{
#ifdef _WIN32
			Sleep(1000);
#else
			sleep(1);
#endif // _WIN32

		}

	if (orderList)
	{
		delete[] orderList;
		orderList = NULL;
	}

	if (session_arrary)
	{
		delete[] session_arrary;
		session_arrary = NULL;
	}

	return 0;
}
