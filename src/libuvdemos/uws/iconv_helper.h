/**
 * @file signal_handler.h
 * @brief signal_handler defines
 */

#pragma once

#include <iconv.h>

__inline static
bool unicode_to_utf8(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* Ŀ�ı���, TRANSLIT�������޷�ת�����ַ���������ַ��滻
	 *           IGNORE �������޷�ת���ַ�����*/
	char* encTo = "UTF-8//IGNORE";
	/* Դ���� */
	char* encFrom = "UNICODE";

	/* ���ת�����
	 *@param encTo Ŀ����뷽ʽ
	 *@param encFrom Դ���뷽ʽ
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* ��Ҫת�����ַ��� */
	//printf("inbuf=%s\n", inbuf);

	/* ��ӡ��Ҫת�����ַ����ĳ��� */
	//printf("inlen=%d\n", *inlen);


	/* ����iconv()�������޸�ָ�룬����Ҫ����Դָ�� */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* ����ת��
	 *@param cd iconv_open()�����ľ��
	 *@param srcstart ��Ҫת�����ַ���
	 *@param inlen ��Ż��ж����ַ�û��ת��
	 *@param tempoutbuf ���ת������ַ���
	 *@param outlen ���ת����,tempoutbufʣ��Ŀռ�
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* ���ת������ַ��� */
	//printf("outbuf=%s\n", outbuf);

	//���ת����outbufʣ��Ŀռ�
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* �رվ�� */
	iconv_close(cd);

	return 0;
}
__inline static
bool utf8_to_unicode(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{

	/* Ŀ�ı���, TRANSLIT�������޷�ת�����ַ���������ַ��滻
	 *           IGNORE �������޷�ת���ַ�����*/
	char* encTo = "UNICODE//IGNORE";
	/* Դ���� */
	char* encFrom = "UTF-8";

	/* ���ת�����
	 *@param encTo Ŀ����뷽ʽ
	 *@param encFrom Դ���뷽ʽ
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* ��Ҫת�����ַ��� */
	//printf("inbuf=%s\n", inbuf);

	/* ��ӡ��Ҫת�����ַ����ĳ��� */
	//printf("inlen=%d\n", *inlen);

	/* ����iconv()�������޸�ָ�룬����Ҫ����Դָ�� */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* ����ת��
	 *@param cd iconv_open()�����ľ��
	 *@param srcstart ��Ҫת�����ַ���
	 *@param inlen ��Ż��ж����ַ�û��ת��
	 *@param tempoutbuf ���ת������ַ���
	 *@param outlen ���ת����,tempoutbufʣ��Ŀռ�
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* ���ת������ַ��� */
	//printf("outbuf=%s\n", outbuf);

	//���ת����outbufʣ��Ŀռ�
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* �رվ�� */
	iconv_close(cd);

	return 0;
}
__inline static
bool gb2312_to_utf8(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* Ŀ�ı���, TRANSLIT�������޷�ת�����ַ���������ַ��滻
	 *           IGNORE �������޷�ת���ַ�����*/
	char* encTo = "UTF-8//IGNORE";
	/* Դ���� */
	char* encFrom = "GB2312";

	/* ���ת�����
	 *@param encTo Ŀ����뷽ʽ
	 *@param encFrom Դ���뷽ʽ
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* ��Ҫת�����ַ��� */
	//printf("inbuf=%s\n", inbuf);

	/* ��ӡ��Ҫת�����ַ����ĳ��� */
	//printf("inlen=%d\n", *inlen);


	/* ����iconv()�������޸�ָ�룬����Ҫ����Դָ�� */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* ����ת��
	 *@param cd iconv_open()�����ľ��
	 *@param srcstart ��Ҫת�����ַ���
	 *@param inlen ��Ż��ж����ַ�û��ת��
	 *@param tempoutbuf ���ת������ַ���
	 *@param outlen ���ת����,tempoutbufʣ��Ŀռ�
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* ���ת������ַ��� */
	//printf("outbuf=%s\n", outbuf);

	//���ת����outbufʣ��Ŀռ�
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* �رվ�� */
	iconv_close(cd);

	return 0;
}
__inline static
bool utf8_to_gb2312(char* inbuf, size_t* inlen, char* outbuf, size_t* outlen)
{
	/* Ŀ�ı���, TRANSLIT�������޷�ת�����ַ���������ַ��滻
	 *           IGNORE �������޷�ת���ַ�����*/
	char* encTo = "GB2312//IGNORE";
	/* Դ���� */
	char* encFrom = "UTF-8";

	/* ���ת�����
	 *@param encTo Ŀ����뷽ʽ
	 *@param encFrom Դ���뷽ʽ
	 *
	 * */
	iconv_t cd = iconv_open(encTo, encFrom);
	if (cd == (iconv_t)-1)
	{
		perror("iconv_open");
	}

	/* ��Ҫת�����ַ��� */
	//printf("inbuf=%s\n", inbuf);

	/* ��ӡ��Ҫת�����ַ����ĳ��� */
	//printf("inlen=%d\n", *inlen);

	/* ����iconv()�������޸�ָ�룬����Ҫ����Դָ�� */
	char* tmpin = inbuf;
	char* tmpout = outbuf;
	size_t insize = *inlen;
	size_t outsize = *outlen;

	/* ����ת��
	 *@param cd iconv_open()�����ľ��
	 *@param srcstart ��Ҫת�����ַ���
	 *@param inlen ��Ż��ж����ַ�û��ת��
	 *@param tempoutbuf ���ת������ַ���
	 *@param outlen ���ת����,tempoutbufʣ��Ŀռ�
	 *
	 * */
	size_t ret = iconv(cd, &tmpin, inlen, &tmpout, outlen);
	if (ret == -1)
	{
		perror("iconv");
	}

	/* ���ת������ַ��� */
	//printf("outbuf=%s\n", outbuf);

	//���ת����outbufʣ��Ŀռ�
	//printf("outlen=%d\n", *outlen);

	int i = 0;

	//for (i = 0; i < (outsize - (*outlen)); i++)
	{
		//printf("%2c", outbuf[i]);
		//printf("%x\n", outbuf[i]);
	}

	/* �رվ�� */
	iconv_close(cd);

	return 0;
}
