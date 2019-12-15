/* This is a simple HTTP(S) web server much like Python's SimpleHTTPServer */
#include "uws.h"
#include <App.h>

/* Helpers for this example */
#include "helpers/AsyncFileReader.h"
#include "helpers/AsyncFileStreamer.h"
#include "helpers/Middleware.h"

/* optparse */
#define OPTPARSE_IMPLEMENTATION
#include "helpers/optparse.h"

#include "common.h"
#include "curl_helper.h"
#include "iconv_helper.h"
//#include "signal_handler.h"

#include <map>
#include <thread>
#include <algorithm>

#define ATTACH_ABORT_HANDLE(res) res->onAborted([]() {std::cout << "ABORTED!" << std::endl;})

#define ROUTE_HTTP_HELLO			"/hello"
#define ROUTE_HTTP_INDEX			"/index"
#define ROUTE_HTTP_CHART_DCE		"/chart-dce"
#define ROUTE_HTTP_CHART_DCE_LIST	"/chart-dce-list"
#define ROUTE_HTTP_CHART_CZCE		"/chart-czce"
#define ROUTE_HTTP_CHART_CZCE_LIST	"/chart-czce-list"
#define ROUTE_HTTP_CHART_SHFE		"/chart-shfe"
#define ROUTE_HTTP_CHART_SHFE_LIST	"/chart-shfe-list"
#define ROUTE_HTTP_CHART_CFFEX		"/chart-cffex"
#define ROUTE_HTTP_CHART_CFFEX_LIST	"/chart-cffex-list"

bool is_trade_day(const time_t& tt)
{
	struct tm* ptm = localtime(&tt);
	//printf("weekday=%d\n", ptm->tm_wday);
	if ((ptm->tm_wday >= 1) && (ptm->tm_wday <= 5))
	{
		return true;
	}
	return false;
}
time_t get_last_trade_day(const time_t& tt)
{
	time_t last_tt = tt - 86400;
	struct tm* ptm = localtime(&tt);
	//printf("weekday=%d\n", ptm->tm_wday);
	while (!is_trade_day(last_tt))
	{
		last_tt -= 86400;
	}
	return last_tt;
}
std::string time_t_to_date(const time_t& tt)
{
	struct tm* ptm = localtime(&tt);
	std::string date(16, '\0');
	snprintf(date.data(), date.size(), "%04d%02d%02d\0", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday);
	return date.c_str();
}
void calc_date_list(std::map<std::string, std::string>& umap, const std::string & date, int count)
{
	struct tm tm = { 0 };
	tm.tm_year = std::stoi(date.substr(0, 4)) - 1900;
	tm.tm_mon = std::stoi(date.substr(4, 2)) - 1;
	tm.tm_mday = std::stoi(date.substr(6, 2));
	time_t date_tt = mktime(&tm);
	for (size_t i = 0; i < count; i++)
	{
		if (!is_trade_day(date_tt))
		{
			date_tt = get_last_trade_day(date_tt);
		}
		umap.insert(std::map<std::string, std::string>::value_type(time_t_to_date(date_tt),""));
		date_tt = get_last_trade_day(date_tt);
	}
	
}
std::string dce_chart(const std::string& product_name, const std::string& date, int count, bool bGetCode = false)
{
	std::string T = "";
	std::string X = "";
	std::string L5 = "";
	std::string S5 = "";
	std::string L10 = "";
	std::string S10 = "";
	std::string L20 = "";
	std::string S20 = "";
	std::string L5S = "";
	std::string L10S = "";
	std::string L20S = "";
	std::map<std::string, std::string> umap;
	setlocale(LC_ALL, "chs");
	calc_date_list(umap, date, count);
	X.append("[");
	L5.append("[");
	S5.append("[");
	L10.append("[");
	S10.append("[");
	L20.append("[");
	S20.append("[");
	L5S.append("[");
	L10S.append("[");
	L20S.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./dce.csv");
		file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/dce.csv");
		//file_reader(data, "./" + it.first + "/dce.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "合约代码：(.*?),Date：(.*?),\r\n";
			std::string pattern2 = "总计,(.*?),(.*?),\r\n";
			std::string pattern3 = "(.*?),(.*?),(.*?),(.*?),\r\n";
			std::vector<std::vector<std::string>> svv1;
			std::vector<std::vector<std::string>> svv2;
			std::vector<std::vector<std::string>> svv3;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			//printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			//printf("flag = %d\n", flag);
			int nIndex1 = (-1);
			//printf("svv1->size=%d,svv1->begin()->size=%d\n", svv1.size(), svv1.begin()->size());
			if (svv1.size())
			{
				if (bGetCode)
				{
					std::string code_list("[");
					for (size_t i = 0; i < svv1.at(0).size(); i++)
					{
						if (i > 0)
						{
							code_list.append(",");
						}
						code_list.append("\"").append(svv1.at(0).at(i)).append("\"");
					}
					code_list.append("]");
					return code_list;
				}
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					if (svv1.at(0).at(i).compare(product_name) == 0)
					{
						nIndex1 = i;
						break;
					}
				}
				if (nIndex1 >= 0 && nIndex1 < svv1.at(0).size())
				{
					// 2-多单手数
					// 3-变化手数
					int nChangeIndex = 1;
					T = svv1.at(0).at(nIndex1).c_str();
					//printf("%s,%s\n", svv1.at(0).at(nIndex1).c_str(), svv1.at(1).at(nIndex1).c_str());
					X.append("'").append(it.first).append("',");

					flag = string_regex_find(result, svv3, out.c_str(), pattern3);
					int nIndexAll = 0;
					int nIndexLong = 0;
					int nIndexShort = 0;
					int nIndexCounter = 0;
					for (size_t i = 0; i < svv3.begin()->size(); i++)
					{
						if (svv3.at(0).at(i).compare("名次") == 0)
						{
							if ((nIndex1 * 3) == nIndexCounter)
							{
								nIndexAll = i;
								for (size_t j = nIndexAll + 1; j < svv3.begin()->size(); j++)
								{
									if (nIndexLong == 0 && nIndexShort  == 0 && svv3.at(0).at(j).compare("名次") == 0)
									{
										nIndexLong = j;
									}
									else if (nIndexLong > 0 && nIndexShort == 0 && svv3.at(0).at(j).compare("名次") == 0)
									{
										nIndexShort = j;
										break;
									}
								}
								break;
							}
							nIndexCounter++;
						}
					}
					//printf("flag = %d\n", flag);
					if (svv3.size() && nIndexLong > 0 && nIndexShort > 0)
					{
						//printf("svv3->size=%d,svv3->begin()->size=%d\n", svv3.size(), svv3.begin()->size());
						//int nIndexLong = nIndex1 * 63 + 21;
						//int nIndexShort = nIndex1 * 63 + 42;
						int nSumLong = 0;
						int nSumShort = 0;
						//printf("%s,%s\n", svv3.at(0).at(nIndexLong).c_str(), svv3.at(1).at(nIndexLong).c_str());
						for (size_t i = 1; i <= 5; i++)
						{
							if (svv3.at(0).at(nIndexLong + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[long]i=%d,%s\n", i, svv3.at(2).at(nIndexLong + i).c_str());
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 1; i <= 5; i++)
						{
							if (svv3.at(0).at(nIndexShort + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[short]i=%d,%s\n", i, svv3.at(2).at(nIndexShort + i).c_str());
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						S5.append("'").append(std::to_string(nSumShort)).append("',");
						//持卖量
						L5.append("'").append(std::to_string(nSumLong)).append("',");
						//持买量-持卖量
						L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						for (size_t i = 6; i <= 10; i++)
						{
							if (svv3.at(0).at(nIndexLong + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[long]i=%d,%s\n", i, svv3.at(2).at(nIndexLong + i).c_str());
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 6; i <= 10; i++)
						{
							if (svv3.at(0).at(nIndexShort + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[short]i=%d,%s\n", i, svv3.at(2).at(nIndexShort + i).c_str());
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						L10.append("'").append(std::to_string(nSumLong)).append("',");
						//持卖量
						S10.append("'").append(std::to_string(nSumShort)).append("',");
						//持买量-持卖量
						L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						for (size_t i = 11; i <= 20; i++)
						{
							if (svv3.at(0).at(nIndexLong + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[long]i=%d,%s\n", i, svv3.at(2).at(nIndexLong + i).c_str());
							nSumLong += std::stoi(svv3.at(2).at(nIndexLong + i).c_str());
						}
						for (size_t i = 11; i <= 20; i++)
						{
							if (svv3.at(0).at(nIndexShort + i).compare("名次") == 0)
							{
								break;
							}
							//printf("[short]i=%d,%s\n", i, svv3.at(2).at(nIndexShort + i).c_str());
							nSumShort += std::stoi(svv3.at(2).at(nIndexShort + i).c_str());
						}
						//持买量
						L20.append("'").append(std::to_string(nSumLong)).append("',");
						//持卖量
						S20.append("'").append(std::to_string(nSumShort)).append("',");
						//持买量-持卖量
						L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
					}
				}
			}
		}
	}

	if (X.length() > 1)
	{
		*X.rbegin() = ']';
	}
	else
	{
		X.append("]");
	}
	if (L5.length() > 1)
	{
		*L5.rbegin() = ']';
	}
	else
	{
		L5.append("]");
	}
	if (S5.length() > 1)
	{
		*S5.rbegin() = ']';
	}
	else
	{
		S5.append("]");
	}
	if (L10.length() > 1)
	{
		*L10.rbegin() = ']';
	}
	else
	{
		L10.append("]");
	}
	if (S10.length() > 1)
	{
		*S10.rbegin() = ']';
	}
	else
	{
		S10.append("]");
	}
	if (L20.length() > 1)
	{
		*L20.rbegin() = ']';
	}
	else
	{
		L20.append("]");
	}
	if (S20.length() > 1)
	{
		*S20.rbegin() = ']';
	}
	else
	{
		S20.append("]");
	}

	if (L5S.length() > 1)
	{
		*L5S.rbegin() = ']';
	}
	else
	{
		L5S.append("]");
	}
	if (L10S.length() > 1)
	{
		*L10S.rbegin() = ']';
	}
	else
	{
		L10S.append("]");
	}
	if (L20S.length() > 1)
	{
		*L20S.rbegin() = ']';
	}
	else
	{
		L20S.append("]");
	}
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, "dce", "VVVVVV");
	string_replace_all(temp, X, "XXXXXX");
	string_replace_all(temp, L5, "LLL5LLL");
	string_replace_all(temp, S5, "SSS5SSS");
	string_replace_all(temp, L10, "LLL10LLL");
	string_replace_all(temp, S10, "SSS10SSS");
	string_replace_all(temp, L20, "LLL20LLL");
	string_replace_all(temp, S20, "SSS20SSS");
	string_replace_all(temp, L5S, "LLL5SSS");
	string_replace_all(temp, L10S, "LLL10SSS");
	string_replace_all(temp, L20S, "LLL20SSS");
	//file_writer(temp, T + ".html");
	return temp;
}
std::string czce_chart(const std::string& product_name, const std::string& date, int count, bool bGetCode = false)
{
	std::string T = "";
	std::string X = "";
	std::string L5 = "";
	std::string S5 = "";
	std::string L10 = "";
	std::string S10 = "";
	std::string L20 = "";
	std::string S20 = "";
	std::string L5S = "";
	std::string L10S = "";
	std::string L20S = "";
	std::map<std::string, std::string> umap;
	setlocale(LC_ALL, "chs");
	calc_date_list(umap, date, count);
	X.append("[");
	L5.append("[");
	S5.append("[");
	L10.append("[");
	S10.append("[");
	L20.append("[");
	S20.append("[");
	L5S.append("[");
	L10S.append("[");
	L20S.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./czce.csv");
		file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/czce.csv");
		//file_reader(data, "./" + it.first + "/czce.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\",\"(.*?)\"\r\n";
			std::vector<std::vector<std::string>> svv1;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			string_replace_all(out, "", " ");
			if (bGetCode)
			{
				std::string pattern = "品种：(.*?)日期";
				flag = string_regex_find(result, svv1, out.c_str(), pattern);
				std::string code_list("[");
				if (svv1.size())
				{
					for (size_t i = 0; i < svv1.at(0).size(); i++)
					{
						if (code_list.length() > 1)
						{
							code_list.append(",");
						}
						code_list.append("\"").append(svv1.at(0).at(i)).append("\"");
					}
				}
				pattern = "合约：(.*?)日期";
				flag = string_regex_find(result, svv1, out.c_str(), pattern);
				if (svv1.size())
				{
					for (size_t i = 0; i < svv1.at(0).size(); i++)
					{
						if (code_list.length() > 1)
						{
							code_list.append(",");
						}
						code_list.append("\"").append(svv1.at(0).at(i)).append("\"");
					}
				}
				code_list.append("]");
				return code_list;
			}
			//printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			//printf("flag = %d\n", flag);
			int nIndex1 = (-1);
			//printf("svv1->size=%d,svv1->begin()->size=%d\n", svv1.size(), svv1.begin()->size());
			if (svv1.size())
			{
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					if (svv1.at(0).at(i).find(product_name) != std::string::npos)
					{
						nIndex1 = i;
						break;
					}
				}
				if (nIndex1 >= 0 && nIndex1 < svv1.at(0).size())
				{
					// 2-多单手数
					// 3-变化手数
					int nChangeIndex = 1;
					T = svv1.at(0).at(nIndex1).c_str();
					//printf("nIndex1=%d,%s,%s\n", nIndex1, svv1.at(0).at(nIndex1).c_str(), svv1.at(1).at(nIndex1).c_str());
					X.append("'").append(it.first).append("',");

					int nSumLong = 0;
					int nSumShort = 0;
					nIndex1 = nIndex1 + 1;
					for (size_t i = nIndex1 + 1; i < svv1.at(0).size(); i++)
					{
						if (svv1.at(0).at(i).compare(0, strlen("合计"), "合计") == 0)
						{
							if (nIndex1 + 5 > i)
							{
								//持买量
								S5.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L5.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 10 > i)
							{
								//持买量
								S10.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L10.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 20 > i)
							{
								//持买量
								S20.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L20.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							break;
						}
						//std::cout << i << ",7," << svv1.at(7).at(i).c_str() << std::endl;
						//std::cout << i << ",11," << svv1.at(11).at(i).c_str() << std::endl;
						std::string sumLong = svv1.at(5).at(i);
						string_replace_all(sumLong, "", ",");
						string_replace_all(sumLong, "", " ");
						std::string sumShort = svv1.at(8).at(i);
						string_replace_all(sumShort, "", ",");
						string_replace_all(sumShort, "", " ");
						nSumLong += std::stoi(sumLong);
						nSumShort += std::stoi(sumShort);
						if (nIndex1 + 5 == i)
						{
							//持买量
							S5.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L5.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 10 == i)
						{
							//持买量
							S10.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L10.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 20 == i)
						{
							//持买量
							S20.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L20.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
					}
				}
			}
		}
	}

	if (X.length() > 1)
	{
		*X.rbegin() = ']';
	}
	else
	{
		X.append("]");
	}
	if (L5.length() > 1)
	{
		*L5.rbegin() = ']';
	}
	else
	{
		L5.append("]");
	}
	if (S5.length() > 1)
	{
		*S5.rbegin() = ']';
	}
	else
	{
		S5.append("]");
	}
	if (L10.length() > 1)
	{
		*L10.rbegin() = ']';
	}
	else
	{
		L10.append("]");
	}
	if (S10.length() > 1)
	{
		*S10.rbegin() = ']';
	}
	else
	{
		S10.append("]");
	}
	if (L20.length() > 1)
	{
		*L20.rbegin() = ']';
	}
	else
	{
		L20.append("]");
	}
	if (S20.length() > 1)
	{
		*S20.rbegin() = ']';
	}
	else
	{
		S20.append("]");
	}

	if (L5S.length() > 1)
	{
		*L5S.rbegin() = ']';
	}
	else
	{
		L5S.append("]");
	}
	if (L10S.length() > 1)
	{
		*L10S.rbegin() = ']';
	}
	else
	{
		L10S.append("]");
	}
	if (L20S.length() > 1)
	{
		*L20S.rbegin() = ']';
	}
	else
	{
		L20S.append("]");
	}
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, "czce", "VVVVVV");
	string_replace_all(temp, X, "XXXXXX");
	string_replace_all(temp, L5, "LLL5LLL");
	string_replace_all(temp, S5, "SSS5SSS");
	string_replace_all(temp, L10, "LLL10LLL");
	string_replace_all(temp, S10, "SSS10SSS");
	string_replace_all(temp, L20, "LLL20LLL");
	string_replace_all(temp, S20, "SSS20SSS");
	string_replace_all(temp, L5S, "LLL5SSS");
	string_replace_all(temp, L10S, "LLL10SSS");
	string_replace_all(temp, L20S, "LLL20SSS");
	//file_writer(temp, T + ".html");
	return temp;
}

std::string shfe_chart(const std::string& product_name, const std::string& date, int count, bool bGetCode = false)
{
	std::string T = "";
	std::string X = "";
	std::string L5 = "";
	std::string S5 = "";
	std::string L10 = "";
	std::string S10 = "";
	std::string L20 = "";
	std::string S20 = "";
	std::string L5S = "";
	std::string L10S = "";
	std::string L20S = "";
	std::map<std::string, std::string> umap;
	setlocale(LC_ALL, "chs");
	calc_date_list(umap, date, count);
	X.append("[");
	L5.append("[");
	S5.append("[");
	L10.append("[");
	S10.append("[");
	L20.append("[");
	S20.append("[");
	L5S.append("[");
	L10S.append("[");
	L20S.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./dce.csv");
		file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/shfe.csv");
		//file_reader(data, "./" + it.first + "/dce.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\r\n";
			std::vector<std::vector<std::string>> svv1;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			string_replace_all(out, "", " ");
			if (bGetCode)
			{
				std::string pattern = "合计:(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\r\n";
				flag = string_regex_find(result, svv1, out.c_str(), pattern);
				std::string code_list("[");
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					if (i > 0)
					{
						code_list.append(",");
					}
					code_list.append("\"").append(svv1.at(0).at(i)).append("\"");
				}
				code_list.append("]");
				return code_list;
			}
			//printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			//printf("flag = %d\n", flag);
			int nIndex1 = (-1);
			//printf("svv1->size=%d,svv1->begin()->size=%d\n", svv1.size(), svv1.begin()->size());
			if (svv1.size())
			{
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					if (svv1.at(0).at(i).find(product_name) != std::string::npos)
					{
						nIndex1 = i;
						break;
					}
				}
				if (nIndex1 >= 0 && nIndex1 < svv1.at(0).size())
				{
					// 2-多单手数
					// 3-变化手数
					int nChangeIndex = 1;
					T = svv1.at(0).at(nIndex1).c_str();
					//printf("nIndex1=%d,%s,%s\n", nIndex1, svv1.at(0).at(nIndex1).c_str(), svv1.at(1).at(nIndex1).c_str());
					X.append("'").append(it.first).append("',");

					int nSumLong = 0;
					int nSumShort = 0;
					for (size_t i = nIndex1 + 1; i < svv1.at(0).size(); i++)
					{
						if (svv1.at(0).at(i).compare(0, strlen("合计"), "合计") == 0)
						{
							if (nIndex1 + 5 > i)
							{
								//持买量
								S5.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L5.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 10 > i)
							{
								//持买量
								S10.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L10.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 20 > i)
							{
								//持买量
								S20.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L20.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							break;
						}
						//std::cout << i << ",7," << svv1.at(7).at(i).c_str() << std::endl;
						//std::cout << i << ",11," << svv1.at(11).at(i).c_str() << std::endl;
						nSumLong += std::stoi(svv1.at(7).at(i).c_str());
						nSumShort += std::stoi(svv1.at(11).at(i).c_str());
						if (nIndex1 + 5 == i)
						{
							//持买量
							S5.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L5.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 10 == i)
						{
							//持买量
							S10.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L10.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 20 == i)
						{
							//持买量
							S20.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L20.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
					}
				}
			}
		}
	}

	if (X.length() > 1)
	{
		*X.rbegin() = ']';
	}
	else
	{
		X.append("]");
	}
	if (L5.length() > 1)
	{
		*L5.rbegin() = ']';
	}
	else
	{
		L5.append("]");
	}
	if (S5.length() > 1)
	{
		*S5.rbegin() = ']';
	}
	else
	{
		S5.append("]");
	}
	if (L10.length() > 1)
	{
		*L10.rbegin() = ']';
	}
	else
	{
		L10.append("]");
	}
	if (S10.length() > 1)
	{
		*S10.rbegin() = ']';
	}
	else
	{
		S10.append("]");
	}
	if (L20.length() > 1)
	{
		*L20.rbegin() = ']';
	}
	else
	{
		L20.append("]");
	}
	if (S20.length() > 1)
	{
		*S20.rbegin() = ']';
	}
	else
	{
		S20.append("]");
	}

	if (L5S.length() > 1)
	{
		*L5S.rbegin() = ']';
	}
	else
	{
		L5S.append("]");
	}
	if (L10S.length() > 1)
	{
		*L10S.rbegin() = ']';
	}
	else
	{
		L10S.append("]");
	}
	if (L20S.length() > 1)
	{
		*L20S.rbegin() = ']';
	}
	else
	{
		L20S.append("]");
	}
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, "shfe", "VVVVVV");
	string_replace_all(temp, X, "XXXXXX");
	string_replace_all(temp, L5, "LLL5LLL");
	string_replace_all(temp, S5, "SSS5SSS");
	string_replace_all(temp, L10, "LLL10LLL");
	string_replace_all(temp, S10, "SSS10SSS");
	string_replace_all(temp, L20, "LLL20LLL");
	string_replace_all(temp, S20, "SSS20SSS");
	string_replace_all(temp, L5S, "LLL5SSS");
	string_replace_all(temp, L10S, "LLL10SSS");
	string_replace_all(temp, L20S, "LLL20SSS");
	//file_writer(temp, T + ".html");
	return temp;
}

std::string cffex_chart(const std::string& product_name, const std::string& date, int count, bool bGetCode = false)
{
	std::string T = "";
	std::string X = "";
	std::string L5 = "";
	std::string S5 = "";
	std::string L10 = "";
	std::string S10 = "";
	std::string L20 = "";
	std::string S20 = "";
	std::string L5S = "";
	std::string L10S = "";
	std::string L20S = "";
	std::map<std::string, std::string> umap;
	setlocale(LC_ALL, "chs");
	calc_date_list(umap, date, count);
	X.append("[");
	L5.append("[");
	S5.append("[");
	L10.append("[");
	S10.append("[");
	L20.append("[");
	S20.append("[");
	L5S.append("[");
	L10S.append("[");
	L20S.append("[");
	for (auto& it : umap)
	{
		std::string data("");
		//file_reader(data, "./cffex.csv");
		file_reader(data, "/usr/share/nginx/html/foot-wash/storage/app/images/edc/" + it.first + "/cffex.csv");
		//file_reader(data, "./" + it.first + "/cffex.csv");
		if (data.length() > 0)
		{
			bool flag = false;
			std::string result("");
			std::string pattern1 = "(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?),(.*?)\n";
			std::vector<std::vector<std::string>> svv1;
			std::string out(data.size() * 4, '\0');
			size_t in_len = data.size();
			size_t out_len = out.size();
			flag = gb2312_to_utf8((char*)data.c_str(), &in_len, (char*)out.c_str(), &out_len);
			string_replace_all(out, "", " ");
			if (bGetCode)
			{
				std::string pattern = ",(.*?),期货公司";
				flag = string_regex_find(result, svv1, out.c_str(), pattern);
				std::string code_list("[");
				if (svv1.size())
				{
					for (size_t i = 0; i < svv1.at(0).size(); i++)
					{
						if (code_list.length() > 1)
						{
							code_list.append(",");
						}
						code_list.append("\"").append(svv1.at(0).at(i)).append("\"");
					}
				}
				code_list.append("]");
				return code_list;
			}
			//printf("flag = %d\n", flag);
			flag = string_regex_find(result, svv1, out.c_str(), pattern1);
			//printf("flag = %d\n", flag);
			int nIndex1 = (-1);
			//printf("svv1->size=%d,svv1->begin()->size=%d\n", svv1.size(), svv1.begin()->size());
			if (svv1.size())
			{
				for (size_t i = 0; i < svv1.at(0).size(); i++)
				{
					printf("%s--%s--%s\n", svv1.at(0).at(i).c_str(), svv1.at(1).at(i).c_str(), svv1.at(2).at(i).c_str());
					if (svv1.at(0).at(i).compare(it.first) == 0 && 
						svv1.at(2).at(i).at(0) >= '1' && 
						svv1.at(2).at(i).at(0) <= '9' &&
						svv1.at(1).at(i).find(product_name) != std::string::npos)
					{
						nIndex1 = i;
						break;
					}
				}
				if (nIndex1 >= 0 && nIndex1 < svv1.at(0).size())
				{
					// 2-多单手数
					// 3-变化手数
					int nChangeIndex = 1;
					T = svv1.at(0).at(nIndex1).c_str();
					//printf("nIndex1=%d,%s,%s\n", nIndex1, svv1.at(0).at(nIndex1).c_str(), svv1.at(1).at(nIndex1).c_str());
					X.append("'").append(it.first).append("',");

					int nSumLong = 0;
					int nSumShort = 0;
					nIndex1 = nIndex1 - 1;
					for (size_t i = nIndex1 + 1; i < svv1.at(0).size(); i++)
					{
						if (svv1.at(1).at(i).compare(0, strlen(product_name.c_str()), product_name.c_str()) != 0)
						{
							if (nIndex1 + 5 > i)
							{
								//持买量
								S5.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L5.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 10 > i)
							{
								//持买量
								S10.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L10.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							else if (nIndex1 + 20 > i)
							{
								//持买量
								S20.append("'").append(std::to_string(nSumShort)).append("',");
								//持卖量
								L20.append("'").append(std::to_string(nSumLong)).append("',");
								//持买量-持卖量
								L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
							}
							break;
						}
						//std::cout << i << ",7," << svv1.at(7).at(i).c_str() << std::endl;
						//std::cout << i << ",11," << svv1.at(11).at(i).c_str() << std::endl;
						std::string sumLong = svv1.at(7).at(i);
						string_replace_all(sumLong, "", ",");
						string_replace_all(sumLong, "", " ");
						std::string sumShort = svv1.at(10).at(i);
						string_replace_all(sumShort, "", ",");
						string_replace_all(sumShort, "", " ");
						nSumLong += std::stoi(sumLong);
						nSumShort += std::stoi(sumShort);
						if (nIndex1 + 5 == i)
						{
							//持买量
							S5.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L5.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L5S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 10 == i)
						{
							//持买量
							S10.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L10.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L10S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
						else if (nIndex1 + 20 == i)
						{
							//持买量
							S20.append("'").append(std::to_string(nSumShort)).append("',");
							//持卖量
							L20.append("'").append(std::to_string(nSumLong)).append("',");
							//持买量-持卖量
							L20S.append("'").append(std::to_string(nSumLong - nSumShort)).append("',");
						}
					}
				}
			}
		}
	}

	if (X.length() > 1)
	{
		*X.rbegin() = ']';
	}
	else
	{
		X.append("]");
	}
	if (L5.length() > 1)
	{
		*L5.rbegin() = ']';
	}
	else
	{
		L5.append("]");
	}
	if (S5.length() > 1)
	{
		*S5.rbegin() = ']';
	}
	else
	{
		S5.append("]");
	}
	if (L10.length() > 1)
	{
		*L10.rbegin() = ']';
	}
	else
	{
		L10.append("]");
	}
	if (S10.length() > 1)
	{
		*S10.rbegin() = ']';
	}
	else
	{
		S10.append("]");
	}
	if (L20.length() > 1)
	{
		*L20.rbegin() = ']';
	}
	else
	{
		L20.append("]");
	}
	if (S20.length() > 1)
	{
		*S20.rbegin() = ']';
	}
	else
	{
		S20.append("]");
	}

	if (L5S.length() > 1)
	{
		*L5S.rbegin() = ']';
	}
	else
	{
		L5S.append("]");
	}
	if (L10S.length() > 1)
	{
		*L10S.rbegin() = ']';
	}
	else
	{
		L10S.append("]");
	}
	if (L20S.length() > 1)
	{
		*L20S.rbegin() = ']';
	}
	else
	{
		L20S.append("]");
	}
	std::string temp;
	file_reader(temp, "chart.html");
	string_replace_all(temp, "cffex", "VVVVVV");
	string_replace_all(temp, X, "XXXXXX");
	string_replace_all(temp, L5, "LLL5LLL");
	string_replace_all(temp, S5, "SSS5SSS");
	string_replace_all(temp, L10, "LLL10LLL");
	string_replace_all(temp, S10, "SSS10SSS");
	string_replace_all(temp, L20, "LLL20LLL");
	string_replace_all(temp, S20, "SSS20SSS");
	string_replace_all(temp, L5S, "LLL5SSS");
	string_replace_all(temp, L10S, "LLL10SSS");
	string_replace_all(temp, L20S, "LLL20SSS");
	//file_writer(temp, T + ".html");
	return temp;
}

#include <rapidjson/rapidjson.h>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/document.h>
__inline static
std::string JSON_VALUE_2_STRING(rapidjson::Value& v)
{
	rapidjson::StringBuffer sb;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sb);
	return (v.Accept(writer) ? std::string(sb.GetString(), sb.GetSize()) : (""));
}
__inline static
rapidjson::Value& STRING_2_JSON_VALUE(rapidjson::Document& d, const std::string& s)
{
	rapidjson::Value& v = d.SetObject();
	d.Parse(s.c_str(), s.size());
	return v;
}
__inline static 
void data_to_json(rapidjson::Value& v, const std::string& data, rapidjson::Document::AllocatorType & allocator)
{
	std::string name = string_reader(data, "; name=\"", "\"");
	std::string filename = string_reader(data, "; filename=\"", "\"");
	if (name.length() > 0 && filename.length() == 0)
	{
		v.AddMember(rapidjson::Document().SetString(name.c_str(), name.length(), allocator), "", allocator);
		if (data.find("Content-Type") == std::string::npos)
		{
			std::string text = string_reader(data, "; name=\"" + name + "\"\r\n", "\r\n");
			v[name.c_str()].SetString(text.c_str(), text.length(), allocator);
		}
	}
	else if (name.length() > 0 && filename.length() > 0)
	{
		v.AddMember(rapidjson::Document().SetString(name.c_str(), name.length(), allocator), "", allocator);
		v[name.c_str()].SetString(filename.c_str(), filename.length(), allocator);
	}
	else if (name.length() == 0 && filename.length() > 0)
	{
		v.AddMember(rapidjson::Document().SetString(filename.c_str(), filename.length(), allocator), "", allocator);
		v[filename.c_str()].SetString(name.c_str(), name.length(), allocator);
	}
	else
	{
		//none
	}
}
__inline static
rapidjson::Value& body_to_json(rapidjson::Document& d, const std::string& strData, const std::string& strSplitter, std::string::size_type stPos = 0)
{
	std::string tmp = ("");
	rapidjson::Value & v = d.SetObject();
	std::string::size_type stIdx = 0;
	std::string::size_type stSize = strData.length();
	if (d.Parse(strData.c_str()).HasParseError())
	{
		while ((stPos = strData.find(strSplitter, stIdx)) != std::string::npos)
		{
			tmp = strData.substr(stIdx, stPos - stIdx);
			if (tmp.length() > 0)
			{
				data_to_json(v, tmp, d.GetAllocator());
			}

			stIdx = stPos + strSplitter.length();
		}

		if (stIdx < stSize)
		{
			tmp = strData.substr(stIdx, stSize - stIdx);
			if (tmp.length() > 0)
			{
				data_to_json(v, tmp, d.GetAllocator());
			}
		}
	}
	return v;
}
std::string url_decode(const std::string& encode)
{
	//std::string encode = "%25aa%E6%A3%89%E8%8A%B1CF";
	std::string decode = "";
	std::string::size_type start_pos = 0;
	std::string::size_type final_pos = 0;
	std::string::size_type count_pos = encode.size();
	while (final_pos < count_pos)
	{
		if ((start_pos = encode.find('%', final_pos)) != std::string::npos)
		{
			decode.append(encode.substr(final_pos, start_pos - final_pos));
			final_pos = start_pos;
			if (final_pos + 3 < count_pos)
			{
				char ch1 = encode.at(final_pos + 1);
				char ch2 = encode.at(final_pos + 2);
				if ((
					(ch1 >= '0' && ch1 <= '9') ||
					(ch1 >= 'a' && ch1 <= 'f') ||
					(ch1 >= 'A' && ch1 <= 'F'))
					&&
					(
					(ch2 >= '0' && ch2 <= '9') ||
						(ch2 >= 'a' && ch2 <= 'f') ||
						(ch2 >= 'A' && ch2 <= 'F'))
					)
				{
					decode.append(1, (uint8_t)std::stoi(encode.substr(final_pos + 1, 2), nullptr, 0x10));
				}
				else
				{
					decode.append(encode.substr(final_pos, 3));
				}
				final_pos = final_pos + 3;
				continue;
			}
		}

		decode.append(encode.substr(final_pos));
		break;
	}
	return decode;
}
int main(int argc, char** argv)
{
	//test_chart();
	//return 0;
	int option;
	struct optparse options;
	optparse_init(&options, argv);
	/* ws->getUserData returns one of these */
	struct PerSocketData {

	};
	struct optparse_long longopts[] = {
		{"port", 'p', OPTPARSE_REQUIRED},
		{"help", 'h', OPTPARSE_NONE},
		{"passphrase", 'a', OPTPARSE_REQUIRED},
		{"key", 'k', OPTPARSE_REQUIRED},
		{"cert", 'c', OPTPARSE_REQUIRED},
		{"dh_params", 'd', OPTPARSE_REQUIRED},
		{0}
	};

	int port = 3000;
	struct us_socket_context_options_t ssl_options = {};
	/* Either serve over HTTP or HTTPS */
	struct us_socket_context_options_t empty_ssl_options = {};

	while ((option = optparse_long(&options, longopts, nullptr)) != -1) {
		switch (option) {
		case 'p':
			port = atoi(options.optarg);
			break;
		case 'a':
			ssl_options.passphrase = options.optarg;
			break;
		case 'c':
			ssl_options.cert_file_name = options.optarg;
			break;
		case 'k':
			ssl_options.key_file_name = options.optarg;
			break;
		case 'd':
			ssl_options.dh_params_file_name = options.optarg;
			break;
		case 'h':
		case '?':
		fail:
			std::cout << "Usage: " << argv[0] << " [--help] [--port <port>] [--key <ssl key>] [--cert <ssl cert>] [--passphrase <ssl key passphrase>] [--dh_params <ssl dh params file>] <public root>" << std::endl;
			return 0;
		}
	}

	char* root = optparse_arg(&options);
	if (!root) {
		goto fail;
	}

	/* Simple echo websocket server, using multiple threads */
	std::vector<std::shared_ptr<std::thread>> threads(std::thread::hardware_concurrency());
	
	std::transform(threads.begin(), threads.end(), threads.begin(), [&](std::shared_ptr<std::thread> t) {
		
		return std::make_shared<std::thread>([&]() {
			AsyncFileStreamer asyncFileStreamer(root);
			if (memcmp(&ssl_options, &empty_ssl_options, sizeof(empty_ssl_options))) {
				/* HTTPS */
				uWS::SSLApp(ssl_options).ws<PerSocketData>("/*", {
						// Settings
						.compression = uWS::SHARED_COMPRESSOR,
						.maxPayloadLength = 16 * 1024,
						.idleTimeout = 10,
						.maxBackpressure = 1 * 1024 * 1204,
						// Handlers
						.open = [](auto* ws, auto* req) {
							std::cout << "open" << std::endl;
						},
						.message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
							std::cout << "message" << std::endl;
							ws->send(message, opCode);
						},
						.drain = [](auto* ws) {
							// Check getBufferedAmount here
							std::cout << "drain" << std::endl;
						},
						.ping = [](auto* ws) {
							std::cout << "ping" << std::endl;
						},
						.pong = [](auto* ws) {
							std::cout << "pong" << std::endl;
						},
						.close = [](auto* ws, int code, std::string_view message) {
							std::cout << "Close:" << code << std::endl;
						}
					})
					.get("/*", [&asyncFileStreamer](auto* res, auto* req) {
						ATTACH_ABORT_HANDLE(res);
						serveFile(res, req);
						asyncFileStreamer.streamFile(res, req->getUrl());
					})
					.post("/*", [](auto* res, auto* req) {
						ATTACH_ABORT_HANDLE(res);
						std::string_view url = req->getUrl();
						printf("==%.*s\n", url.length(), url.data());
						std::string_view query = req->getQuery();
						printf("==%.*s\n", query.length(), query.data());
						std::string_view param0 = req->getParameter(0);
						printf("==%.*s\n", param0.length(), param0.data());
						std::string_view param1 = req->getParameter(1);
						printf("==%.*s\n", param1.length(), param1.data());

						/* Allocate automatic, stack, variable as usual */
						std::string buffer("");
						/* Move it to storage of lambda */
						res->onData([res, buffer = std::move(buffer)](std::string_view data, bool last) mutable {
							/* Mutate the captured data */
							buffer.append(data.data(), data.length());

							if (last) {
								/* Use the data */
								std::cout << "We got all data, length: " << buffer.length() << std::endl;
								std::cout << "data=" << buffer << std::endl;
								//us_listen_socket_close(listen_socket);
								//res->end("Thanks for the data!");
								{
									std::string data(buffer.c_str(), buffer.length());
									file_writer(data, "out.txt");
									std::string value = string_reader(data, "----------------------------", "\r\n");
									std::string flags = "----------------------------" + value;
									std::cout << "#######################################################" << std::endl;
									std::cout << "value=" << value << std::endl;

									rapidjson::Document d;
									rapidjson::Value& v = body_to_json(d, data, flags + "\r\n");
									printf("value = %s\n", JSON_VALUE_2_STRING(v).c_str());
								}
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("[OK]");
								std::cout << "end req" << std::endl;
								/* When this socket dies (times out) it will RAII release everything */
							}
						});
						/* Unwind stack, delete buffer, will just skip (heap) destruction since it was moved */
							})
					.listen(port, [port, root](auto* token) {
						if (token) {
							std::cout << "Serving " << root << " over HTTPS a " << port << std::endl;
							std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << port << std::endl;
						}
						else {
							std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port " << port << std::endl;
						}
					}).run();
			}
			else {
				/* HTTP */
				uWS::App()
					.ws<PerSocketData>("/*", {
						// Settings
						.compression = uWS::SHARED_COMPRESSOR,
						.maxPayloadLength = 16 * 1024,
						.idleTimeout = 10,
						.maxBackpressure = 1 * 1024 * 1204,
						// Handlers
						.open = [](auto* ws, auto* req) {
							std::cout << "open" << std::endl;
						},
						.message = [](auto* ws, std::string_view message, uWS::OpCode opCode) {
							std::cout << "message" << std::endl;
							ws->send(message, opCode);
						},
						.drain = [](auto* ws) {
							// Check getBufferedAmount here
							std::cout << "drain" << std::endl;
						},
						.ping = [](auto* ws) {
							std::cout << "ping" << std::endl;
						},
						.pong = [](auto* ws) {
							std::cout << "pong" << std::endl;
						},
						.close = [](auto* ws, int code, std::string_view message) {
							std::cout << "Close:" << code << std::endl;
						}
					})
					.get("/*", [&asyncFileStreamer, root](auto* res, auto* req) {
						ATTACH_ABORT_HANDLE(res);

						std::string_view url = req->getUrl();
							printf("==%.*s\n", url.length(), url.data());
						if (std::string(url.data(), url.length()).compare(0, strlen(ROUTE_HTTP_HELLO), ROUTE_HTTP_HELLO) == 0)
						{
							/* You can efficiently stream huge files too */
							res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("Hello HTTP!");
						}
						else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_DCE) == 0)
						{
							//printf("==%.*s\n", url.length(), url.data());
							std::string_view query = req->getQuery();
							//printf("==%.*s\n", query.length(), query.data());
							std::string product_name = "";
							std::string date = "";
							std::string days = "";
							std::string result = "";
							std::vector<std::vector<std::string>> svv;
							std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
							string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
							if (svv.size() > 0)
							{
								product_name = svv.at(0).at(0);
								date = svv.at(1).at(0);
								days = svv.at(2).at(0);
								try
								{
									/*if (std::stoi(days) > 31)
									{
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!days cannot more than 31");
									}
									else
									{
										//printf("{==%.*s==%.*s==%.*s}\n", product_name.length(), product_name.data(), date.length(), date.data(), days.length(), days.data());
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
									}*/
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
								}
								catch (const std::exception & e)
								{
									std::cout << "Exception:" << e.what() << std::endl;
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
							}
							else
							{
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
							}
							std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
							std::cout << "Printing took "
								<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
								<< "us.\n";
						}
						else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_DCE_LIST) == 0)
						{
							//printf("==%.*s\n", url.length(), url.data());
							std::string_view query = req->getQuery();
							//printf("==%.*s\n", query.length(), query.data());
							std::string product_name = "";
							std::string date = "";
							std::string days = "";
							std::string result = "";
							std::vector<std::vector<std::string>> svv;
							std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
							string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
							if (svv.size() > 0)
							{
								product_name = svv.at(0).at(0);
								date = svv.at(1).at(0);
								days = svv.at(2).at(0);
								try
								{
									/*if (std::stoi(days) > 31)
									{
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!days cannot more than 31");
									}
									else
									{
										//printf("{==%.*s==%.*s==%.*s}\n", product_name.length(), product_name.data(), date.length(), date.data(), days.length(), days.data());
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
									}*/
									res->writeHeader("Content-Type", "application/json; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days), true).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_CZCE) == 0)
							{
								printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = url_decode(svv.at(0).at(0));
									
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);

									//printf("==%s==%s==%s\n", product_name.data(), date.data(), days.data());
									try
									{
										/*if (std::stoi(days) > 31)
										{
											res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!days cannot more than 31");
										}
										else
										{
											//printf("{==%.*s==%.*s==%.*s}\n", product_name.length(), product_name.data(), date.length(), date.data(), days.length(), days.data());
											res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
										}*/
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(czce_chart(product_name, date, std::stoi(days)).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_CZCE_LIST) == 0)
							{
								//printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = svv.at(0).at(0);
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);
									try
									{
										/*if (std::stoi(days) > 31)
										{
											res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!days cannot more than 31");
										}
										else
										{
											//printf("{==%.*s==%.*s==%.*s}\n", product_name.length(), product_name.data(), date.length(), date.data(), days.length(), days.data());
											res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(dce_chart(product_name, date, std::stoi(days)).c_str());
										}*/
										res->writeHeader("Content-Type", "application/json; charset=utf-8")->end(czce_chart(product_name, date, std::stoi(days), true).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_SHFE) == 0)
							{
								//printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = svv.at(0).at(0);
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);
									try
									{
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(shfe_chart(product_name, date, std::stoi(days)).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_SHFE_LIST) == 0)
							{
								//printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = svv.at(0).at(0);
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);
									try
									{
										res->writeHeader("Content-Type", "application/json; charset=utf-8")->end(shfe_chart(product_name, date, std::stoi(days), true).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_CFFEX) == 0)
							{
								//printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = svv.at(0).at(0);
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);
									try
									{
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(cffex_chart(product_name, date, std::stoi(days)).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_CHART_CFFEX_LIST) == 0)
							{
								//printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string product_name = "";
								std::string date = "";
								std::string days = "";
								std::string result = "";
								std::vector<std::vector<std::string>> svv;
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								string_regex_find(result, svv, std::string(query.data(), query.length()), "p=(.*?)&d=(.*?)&c=(.*+)");
								if (svv.size() > 0)
								{
									product_name = svv.at(0).at(0);
									date = svv.at(1).at(0);
									days = svv.at(2).at(0);
									try
									{
										res->writeHeader("Content-Type", "application/json; charset=utf-8")->end(cffex_chart(product_name, date, std::stoi(days), true).c_str());
									}
									catch (const std::exception & e)
									{
										std::cout << "Exception:" << e.what() << std::endl;
										res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
									}
								}
								else
								{
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("param error!");
								}
								std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
								std::cout << "Printing took "
									<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
									<< "us.\n";
							}
							else if (std::string(url.data(), url.length()).compare(0, url.length(), ROUTE_HTTP_INDEX) == 0)
							{
								printf("==%.*s\n", url.length(), url.data());
								std::string_view query = req->getQuery();
								//printf("==%.*s\n", query.length(), query.data());
								std::string result = "";
								std::chrono::steady_clock::time_point s = std::chrono::steady_clock::now();
								file_reader(result, root + std::string(url.data(), url.length()) + ".html");
								if (result.length() <= 0)
								{
#define FMT_404 "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\"><html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL %.*s was not found on this server.</p></body></html>"
									char RES_404[SHRT_MAX] = { 0 };
									std::cout << "Did not find file: " << url << std::endl;
									snprintf(RES_404, sizeof(RES_404), FMT_404, url.length(), url.data());
#undef FMT_404
									res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(std::string_view(RES_404, strlen(RES_404)));
								}
							else
							{
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end(result);
							}
							std::chrono::steady_clock::time_point e = std::chrono::steady_clock::now();
							std::cout << "Printing took "
								<< std::chrono::duration_cast<std::chrono::microseconds>(e - s).count()
								<< "us.\n";
						}
						else
						{
							serveFile(res, req);
							asyncFileStreamer.streamFile(res, req->getUrl());
						}
					})
					.post("/*", [](auto* res, auto* req) {
						ATTACH_ABORT_HANDLE(res);
						std::string_view url = req->getUrl();
						printf("==%.*s\n", url.length(), url.data());
						std::string_view query = req->getQuery();
						printf("==%.*s\n", query.length(), query.data());
						std::string_view param0 = req->getParameter(0);
						printf("==%.*s\n", param0.length(), param0.data());
						std::string_view param1 = req->getParameter(1);
						printf("==%.*s\n", param1.length(), param1.data());

						/* Allocate automatic, stack, variable as usual */
						std::string buffer("");
						/* Move it to storage of lambda */
						res->onData([res, buffer = std::move(buffer)](std::string_view data, bool last) mutable {
							/* Mutate the captured data */
							buffer.append(data.data(), data.length());

							if (last) {
								/* Use the data */
								std::cout << "We got all data, length: " << buffer.length() << std::endl;
								std::cout << "data=" << buffer << std::endl;
								//us_listen_socket_close(listen_socket);
								//res->end("Thanks for the data!");
								{
									std::string data(buffer.c_str(), buffer.length());
									file_writer(data, "out.txt");
									std::string value = string_reader(data, "----------------------------", "\r\n");
									std::string flags = "----------------------------" + value;
									std::cout << "#######################################################" << std::endl;
									std::cout << "value=" << value << std::endl;

									rapidjson::Document d;
									rapidjson::Value& v = body_to_json(d, data, flags + "\r\n");
									printf("value = %s\n", JSON_VALUE_2_STRING(v).c_str());
								}
								res->writeHeader("Content-Type", "text/html; charset=utf-8")->end("[OK]");
								std::cout << "end req" << std::endl;
								/* When this socket dies (times out) it will RAII release everything */
							}
						});
						/* Unwind stack, delete buffer, will just skip (heap) destruction since it was moved */
					})
					.listen(port, [port, root](auto* token) {
						if (token) {
							std::cout << "Serving " << root << " over HTTP a " << port << std::endl;
							std::cout << "Thread " << std::this_thread::get_id() << " listening on port " << port << std::endl;
						}
						else {
							std::cout << "Thread " << std::this_thread::get_id() << " failed to listen on port " << port << std::endl;
						}
					}).run();
			}
			});
		});
	std::for_each(threads.begin(), threads.end(), [](std::shared_ptr<std::thread> t) {
		t->join();
		});
}
