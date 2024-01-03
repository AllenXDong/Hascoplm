/* 
 @<COPYRIGHT>@
 ==================================================
 Copyright 2012
 Siemens Product Lifecycle Management Software Inc.
 All Rights Reserved.
 ==================================================
 @<COPYRIGHT>@
*/

#include <unidefs.h>
#if defined(SUN)
#include <unistd.h>
#endif

#include <downloadpdfinterface2106impl.hxx>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <tcinit/tcinit.h>
#include <tc/tc_startup.h>
#include <tc/emh.h>
#include <sa/sa.h>
#include <sa/groupmember.h>
#include <sa/role.h>
#include <sa/group.h>
#include <sa/user.h>
#include <tccore/workspaceobject.h>
#include <tccore/item.h>
#include <tc/folder.h>
#include <tccore/aom.h>
#include <form/form.h>
#include <tccore/grm.h>
#include <sub_mgr/standardtceventtypes.h>
#include <sub_mgr/tceventtype.h>
#include <sub_mgr/tcactionhandler.h>
#include <sub_mgr/subscription.h>
#include <tccore/grmtype.h>
#include <tccore/grm.h>
#include <sa/audit.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <ae/dataset.h>
#include <ae/datasettype.h >
#include <epm/epm_toolkit_tc_utils.h>
#include <tccore/workspaceobject.h>
#include <tc/envelope.h>
#include <qry/qry.h>
#include <property/nr.h>
#include <tccore/item.h>
#include <property/propdesc.h>
#include <epm/epm.h>
#include <epm/epm_toolkit_tc_utils.h>
#include <tccore/item.h>
#include <tccore/grmtype.h>
#include <tccore/grm.h>
#include <tccore/ImanType.hxx>
#include <tccore/tctype.h>
#include <sa/am.h>
#include <tccore/aom.h>
#include <tccore/aom_prop.h>
#include <property/prop_errors.h>
#include <tccore/workspaceobject.h>
#include <tc/preferences.h>
#include <sa/sa.h>
#include <ae/ae.h>
#include <bom/bom_attr.h>
#include <bom/bom.h>
#include <fclasses/tc_string.h>
#include <tc/folder.h>
#include <fclasses/tc_date.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <tccore/project.h>
#include <lov/lov.h>
#include <io.h>
#include <direct.h>
#include <algorithm>
#include <list>
#include <codecvt>
#include <fstream>
#include <cfm/cfm.h>


using namespace SK2::Soa::CustomService::_2021_06;
using namespace Teamcenter::Soa::Server;

#define MAX_PATH_LENGTH 2000
#define MAX_PRINTLINE_LENGTH 2000
#define MAX_LINE_LENGTH 65535

using namespace std;
#define DOFREE(obj)								\
{												\
	if(obj)										\
	{											\
		MEM_free(obj);							\
		obj = NULL;								\
	}											\
}

static vector<string> split(const string& str, const string& delim) {

	vector<string> res;
	if (str == "")
		return res;
	//在字符串末尾也加入分隔符，方便截取最后一段
	string strs = str + delim;
	size_t pos = strs.find(delim);

	while (pos != strs.npos)
	{
		string temp = strs.substr(0, pos);
		res.push_back(temp);
		//去掉已分割的字符串,在剩下的字符串中进行分割
		strs = strs.substr(pos + delim.size(), strs.size());
		pos = strs.find(delim);
	}

	return res;
}
static void replaceAll(std::string& str, const std::string& from, const std::string& to) {
		if (from.empty())
			return;
		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
			str.replace(start_pos, from.length(), to);
			start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
		}
	}

static int GetPreferenceValue(const char* cPreference, string& mValue)
	{
		//map<string, string> mValues;
		int rcode = 0;
		char** valuesPref = NULL;
		int count = 0;
		PREF_ask_char_values(cPreference, &count, &valuesPref);

		for (size_t i = 0; i < count; i++)
		{
			mValue = valuesPref[i];
			break;
		}
		DOFREE(valuesPref);
		return rcode;
	}

	static int GetPreferenceValues(const char* cPreference, vector<string>& mValue)
	{
		//map<string, string> mValues;
		int rcode = 0;
		char** valuesPref = NULL;
		int count = 0;
		PREF_ask_char_values(cPreference, &count, &valuesPref);
		for (size_t i = 0; i < count; i++)
		{
			string sValue = valuesPref[i];
			mValue.push_back(sValue);
		}
		DOFREE(valuesPref);
		return rcode;
	}

	static int isExist(const char* filename)
	{
		return (::_access(filename, 0) == 0);
	}
	/*

	static FILE* CreateLogFile(const string log_path)
	{
		FILE* file = NULL;
		char* session_uid = NULL;
		char logFileDir[MAX_PATH_LENGTH];
		tc_strcpy(logFileDir, "");
		tc_strcat(logFileDir, log_path.c_str());

		if (isExist(logFileDir))
		{
			file = fopen(logFileDir, "rb+");
			fseek(file, 0, SEEK_END);
		}
		else
		{
			file = fopen(logFileDir, "wb+");
		}
		if (session_uid != NULL) {
			DOFREE(session_uid);
			session_uid = NULL;
		}
		return file;
	}
*/

	static void writeLog(const char* datasetUID,const char * message)
		{
			tag_t
				dataset = NULLTAG,
				new_text_file = NULL_TAG,
				text_file = NULLTAG;
			IMF_file_t
				file_descriptor = NULL;
			AE_reference_type_t
				ref_type;

				if (!strcmp(datasetUID, "")) return;
				ITK__convert_uid_to_tag(datasetUID, &dataset);
				if (dataset == NULLTAG) return;
				ITKCALL(AOM_refresh(dataset, TRUE));

				ITKCALL(AE_ask_dataset_named_ref2(dataset, "Text",
					&ref_type, &text_file));
				if (text_file == NULLTAG) return;
				ITKCALL(AOM_refresh(text_file, TRUE));

				if (isExist("C:\\TEMP\\before.txt"))
				{
					remove("C:\\TEMP\\before.txt");
				}
				if (isExist("C:\\TEMP\\after.txt"))
				{
					remove("C:\\TEMP\\after.txt");
				}

				ITKCALL(IMF_export_file(text_file, "C:\\TEMP\\before.txt"));


				ITKCALL(IMF_ask_file_descriptor(text_file, &file_descriptor));
				ITKCALL(IMF_open_file(file_descriptor, SS_APPEND));
				ITKCALL(IMF_write_file_line2(file_descriptor, message));
				ITKCALL(IMF_close_file(file_descriptor));
				ITKCALL(AOM_save(text_file));

				ITKCALL(IMF_export_file(text_file, "C:\\TEMP\\after.txt"));

				/*  In order for the changes to be seen in the RAC, a new tag needs to be
					generated.  See PR 1666280.
				*/
				ITKCALL(IMF_replace_file_and_get_new_tag(text_file,
					"C:\\TEMP\\after.txt", TRUE, &new_text_file));
				ITKCALL(AOM_unload(text_file));

				ITKCALL(AE_replace_dataset_named_ref2(dataset, text_file,
					"Text", ref_type, new_text_file));
				ITKCALL(AE_save_myself(dataset));
				ITKCALL(AOM_unload(dataset));

				ITKCALL(AOM_lock_for_delete(text_file));
				ITKCALL(AOM_delete(text_file));

		}

	static tag_t import_7z_file(const string datasetname, const string filePath)
		{
			tag_t
				dataset = NULLTAG,
				ds_type = NULLTAG,
				tool = NULLTAG,
				file = NULLTAG;
			IMF_file_t
				descriptor;

			/* create dataset not using default tool */
			ITKCALL(AE_find_datasettype2("Zip", &ds_type));
			ITKCALL(AE_find_tool2("UnZip", &tool));
			if (tool == NULLTAG)
			{
				exit(0);
			}
			ITKCALL(AE_create_dataset_with_id(ds_type, datasetname.c_str(), datasetname.c_str(), (datasetname + "_id").c_str(), "A", &dataset));
			ITKCALL(AE_set_dataset_tool(dataset, tool));
			ITKCALL(AE_set_dataset_format2(dataset, "BINARY_REF"));
			ITKCALL(AOM_save(dataset));

			/* attach dataset to the item revision */
			//ITKCALL(GRM_find_relation_type("IMAN_specification", &relation_type));
			//ITKCALL(GRM_create_relation(rev, dataset, relation_type, NULL, &relation));
			//ITKCALL(GRM_save_relation(relation));
			//ITKCALL(AOM_unload(rev));

			/* import file */
			ITKCALL(IMF_fmsfile_import(filePath.c_str(), datasetname.c_str(), SS_BINARY, &file, &descriptor));

			/* add ImanFile to dataset */
			ITKCALL(AOM_refresh(dataset, TRUE));
			ITKCALL(AE_add_dataset_named_ref2(dataset, "ZIPFILE", AE_PART_OF, file));
			ITKCALL(AOM_save(dataset));
			ITKCALL(AOM_unload(file));
			ITKCALL(AOM_unload(dataset));
			return dataset;
		}

	static int getRevMasterForm(const tag_t itemRev, tag_t* form_tag)
	{
		int rcode = ITK_ok;
		tag_t* objects = NULL;
		int count = 0;

		/* initialize the form_tag to NULLTAG */
		*form_tag = NULLTAG;
		/* get the master form relation type */
		//ITKCALL(GRM_find_relation_type("IMAN_master_form_rev", &relation_type));
		/* get the relation objects of master form relation type */
		//ITKCALL(GRM_list_secondary_objects_only(itemRev, relation_type, &count, &objects));
		ITKCALL(AOM_ask_value_tags(itemRev, "IMAN_master_form_rev", &count, &objects));
		if ((count == 1) && (objects[0] != NULLTAG))
		{
			/* set the result object */
			*form_tag = objects[0];
		}
		DOFREE(objects);
		return rcode;
	}
	static void current_time(date_t * date_tag)
	{
		time_t ltime;
		struct tm *today;

		// Set time zone from TZ environment variable. If TZ is not set,
		// the operating system is queried to obtain the default value
		// for the variable.
		//
		_tzset();

		// Get UNIX-style time and display as number and string.
		time(&ltime);

		today = localtime(&ltime);
		date_tag->year = (short)today->tm_year + 1900;
		date_tag->month = (byte)today->tm_mon;
		date_tag->day = (byte)today->tm_mday;
		date_tag->hour = (byte)today->tm_hour;
		date_tag->minute = (byte)today->tm_min;
		date_tag->second = (byte)today->tm_sec;
	}



	/**
 *  Desc:  对象是否有release_status
 **/
	static int exist_release_status(const tag_t object_tag,const vector<string>releaseStatusVector, logical *exist) {
		int rcode = ITK_ok;

		int i = 0, k = 0;
		int value_num = 0;
		tag_t * value_list = NULL;

		if(object_tag!=NULLTAG)ITKCALL(AOM_ask_value_tags(object_tag, "release_status_list", &value_num, &value_list));
		*exist = false;

		for (i = 0; i < value_num; i++)
		{
			char *obj_name = NULL;
			ITKCALL(AOM_ask_value_string(value_list[i], "object_name", &obj_name));
			_strlwr(obj_name);
			fprintf(stdout, "release status name = %s\n", obj_name);

			for (k = 0; k < releaseStatusVector.size(); k++)
			{
				if (::_stricmp(obj_name, releaseStatusVector[k].c_str()) == 0)
				{
					*exist = true;
				}
			}
			DOFREE(obj_name);
		}
		DOFREE(value_list);

		return rcode;
	}

	static tag_t getLastREV(const tag_t item, const vector<string>releasestatusList) {
		int lastRev = -1;
		int num = 0;
		tag_t * revs = NULL;
		tag_t return_rev = NULLTAG;
		map<string, int> mp;
		ITKCALL(ITEM_list_all_revs(item, &num, &revs)) //列出所有版本
			for (int i = 0; i < num; i++) {
				char * revid = NULL;
				ITKCALL(AOM_ask_value_string(revs[i], "item_revision_id", &revid));
				mp.insert(make_pair(revid, i));
				DOFREE(revid);
			}
		//map<string, int> ::iterator it;
		//for (it = mp.begin(); it != mp.end(); it++)lastRev = it->second;
		for (map<string, int>::reverse_iterator rit = mp.rbegin(); rit != mp.rend(); rit++)
		{
			bool existReleaseStatus = false;
			tag_t rev_tag = revs[(*rit).second];
			exist_release_status(rev_tag, releasestatusList, &existReleaseStatus);
			if (existReleaseStatus)
			{
				lastRev = (*rit).second;
				break;
			}
		}
		if (lastRev != -1)
		{
			return_rev = revs[lastRev];
		}
		DOFREE(revs);
		return return_rev;
	}

	static int get_iman_specification_pdf_dataset_tag(const tag_t object_tag,  vector<tag_t> &pdf_tag_vector) {
		int rcode = ITK_ok;
		int i = 0;
		int value_num = 0;
		tag_t * value_list = NULL;


		ITKCALL(AOM_ask_value_tags(object_tag, TC_specification_rtype, &value_num, &value_list));

		for (i = 0; i < value_num; i++)
		{
			char *object_type = NULL;
			AOM_ask_value_string(value_list[i], "object_type", &object_type);
			printf("object_type=%s\n", object_type);
			if (tc_strcmp(object_type, "PDF") == 0) {
				pdf_tag_vector.push_back(value_list[i]);
			}
			DOFREE(object_type);
		}

		DOFREE(value_list);

		return rcode;
	}

	static int get_apqp_pdf_dataset_tag(const tag_t object_tag,const string relation, const vector<string>releasestatusList,vector<tag_t> &pdf_tag_vector) {
		int rcode = ITK_ok;

		int ybj_revision_APQP_objects_count = 0;
		tag_t *ybj_revision_APQP_objects_tags = NULL;
		ITKCALL(AOM_ask_value_tags(object_tag, relation.c_str(), &ybj_revision_APQP_objects_count, &ybj_revision_APQP_objects_tags));
		if (ybj_revision_APQP_objects_count > 0)
		{
			for (size_t j = 0; j < ybj_revision_APQP_objects_count; j++)
			{
				char *object_type_temp = NULL;
				ITKCALL(AOM_ask_value_string(ybj_revision_APQP_objects_tags[j], "object_type", &object_type_temp));
				printf("object_type=%s\n", object_type_temp);
				if (tc_strcmp(object_type_temp, "SK2_APQPDoc Revision") == 0 || tc_strcmp(object_type_temp, "SK2_APQPDoc") == 0)
				{
					tag_t APQPDoc_revision_object_tag = NULLTAG;
					if(tc_strcmp(object_type_temp, "SK2_APQPDoc Revision") == 0)
					{
						APQPDoc_revision_object_tag = ybj_revision_APQP_objects_tags[j];
						bool existReleaseStatus = false;
						exist_release_status(APQPDoc_revision_object_tag, releasestatusList, &existReleaseStatus);
						if(!existReleaseStatus) continue;  //判断是否是存在发布状态 20230913新增需求
					}
					else
					{
						APQPDoc_revision_object_tag = getLastREV(ybj_revision_APQP_objects_tags[j], releasestatusList);
						if (APQPDoc_revision_object_tag == NULLTAG) continue; //判断是否是存在发布状态
					}
					tag_t irm_tag = NULLTAG;
					char* ApqpName = NULL;
					getRevMasterForm(APQPDoc_revision_object_tag, &irm_tag);
					ITKCALL(AOM_ask_value_string(irm_tag, "ApqpName", &ApqpName));
					if (tc_strcmp(ApqpName, "19-零部件及材料明细表") == 0 || tc_strcmp(ApqpName, "13-ADVP 或试验大纲") == 0 || tc_strcmp(ApqpName, "38-零部件重要特征值") == 0)
					{
						get_iman_specification_pdf_dataset_tag(APQPDoc_revision_object_tag, pdf_tag_vector);
					}
					DOFREE(ApqpName);
				}
				DOFREE(object_type_temp);
			}
		}
		DOFREE(ybj_revision_APQP_objects_tags);

		return rcode;

	}
	static string downloadAndZipPDF(vector<tag_t>download_dataset_vector,string preference_file_path,string downloadType ,string item_id, string item_revision_id, string user)
		{
			string returnUid = "";
			string downLoadLoadString = "";

			date_t status_now;
			char* date_string = NULL;
			char* file_date_string = NULL;
			char* folder_date_string = NULL;
			char* download_date_string = NULL;
			char* uid = NULL;
			current_time(&status_now);

			if (DATE_date_to_string(status_now, "%Y%m%d", &date_string) != ITK_ok)
			{
				printf("!*ERROR*!: Failed to get current date time\n");
			}
			//2023_01#OA_Send.csv
			if (DATE_date_to_string(status_now, "%Y%m", &file_date_string) != ITK_ok)
			{
				printf("!*ERROR*!: Failed to get current date time\n");
			}
			//2023/8/11|15:10:10|chengbinx|CO20221234|success
			if (DATE_date_to_string(status_now, "%Y/%m/%d|%H:%M:%S", &download_date_string) != ITK_ok)
			{
				printf("!*ERROR*!: Failed to get current date time\n");
			}
			if (DATE_date_to_string(status_now, "%Y%m%d%H%M%S", &folder_date_string) != ITK_ok)
			{
				printf("!*ERROR*!: Failed to get current date time\n");
			}
			string temp_folder = getenv("TMP");

			temp_folder += "\\TempFolder" + string(folder_date_string);
			::_mkdir(temp_folder.c_str());

			string root_folder = getenv("TC_ROOT");
			fprintf(stdout, "data = %s", download_date_string);
			string zipFilName = date_string;
			zipFilName = zipFilName + "_" + item_id + "_" + item_revision_id + ".7z";
			string zipFilePath = temp_folder + "\\" + zipFilName;



			//preference_file_path = preference_file_path + "\\" + file_date_string + "#"+downloadType+".txt";


				for (size_t i = 0; i < download_dataset_vector.size(); i++)
				{
					char* name = NULL;
					string nameStr = "";
					tag_t dataset_tag = download_dataset_vector[i];
					ITKCALL(AOM_ask_name(dataset_tag, &name));
					nameStr = name;
					fprintf(stdout, "start to handler %s\n", name);
					string temp_dataset_path = temp_folder + "\\temp"+to_string(i)+".pdf";
					if (tc_strstr(name, "/") != NULL)
					{
						replaceAll(nameStr, "/", "_");
					}
					string dataset_path = temp_folder + "\\" + nameStr;


					if (tc_strstr(name, ".pdf") != NULL || tc_strstr(name, ".PDF") != NULL)
					{
						//temp_dataset_path.replace(dataset_path.find_last_of("."), 4, "temp.pdf");
					}
					else
					{
						//temp_dataset_path = dataset_path + "temp.pdf";
						dataset_path = dataset_path + ".pdf";
					}
					ITKCALL(AE_export_named_ref(dataset_tag, "PDF_Reference", temp_dataset_path.c_str()));
					string pdfwatermarkcmd = root_folder + "\\bin\\PDFAddWatermark.exe \"" + temp_dataset_path + "\" \"" + dataset_path + "\" " + user;
					system(pdfwatermarkcmd.c_str());
					string pdfzipcmd = root_folder + +"\\bin\\7-Zip\\7z.exe a " + zipFilePath + " \"" + dataset_path+ "\"";
					system(pdfzipcmd.c_str());
					//downLoadLoadString = downLoadLoadString + "|" + name;
					downLoadLoadString = downLoadLoadString + name + "|";
					DOFREE(name);
				}

				tag_t dataset_tag = import_7z_file(zipFilName, zipFilePath);
				string deleteTempFoldercmd = "rd /s /q " + temp_folder;
				system(deleteTempFoldercmd.c_str());
				ITK__convert_tag_to_uid(dataset_tag, &uid);
				returnUid = uid;
				//2023/8/11|15:10:10|chengbinx|CO20221234
				//2 | CO图纸.pdf | 装配图纸.pdf |
				//	下载成功
				//	水印成功
				//	压缩成功
				//	上传成功
				string message = string(download_date_string) + '|' + user + '|' + item_id + '|' + to_string(download_dataset_vector.size()) + '|' + downLoadLoadString + "下载成功|水印成功|压缩成功|上传成功\n";
				writeLog(preference_file_path.c_str(),message.c_str());

				//FILE *logFile = CreateLogFile(preference_file_path);
				//string message = string(download_date_string) + '|' + user + '|' + item_id + '|' + to_string(download_dataset_vector.size()) + '|' + downLoadLoadString + "下载成功|水印成功|压缩成功|上传成功\n";
				//fprintf(logFile, message.c_str());
				//fclose(logFile);

				DOFREE(date_string);
				DOFREE(file_date_string);
				DOFREE(folder_date_string);
				DOFREE(download_date_string);
				DOFREE(uid);

			return returnUid;
		}

	static void printrecursivebom(tag_t line, const vector<string>releasestatusList, vector<tag_t>&downloadVector, const char* inputparameter)
	{
		int n = 0;
		int k = 0;
		int attribute;
		tag_t *children;
		BOM_line_ask_child_lines(line, &n, &children);
		for (k = 0; k < n; k++)
		{
			logical exist = false;
			tag_t item_rev_tag = NULLTAG;
			//get the revision from bomline
			ITKCALL(BOM_line_look_up_attribute(bomAttr_lineItemRevTag, &attribute));
			ITKCALL(BOM_line_ask_attribute_tag(children[k], attribute, &item_rev_tag));
			exist_release_status(item_rev_tag, releasestatusList, &exist);
			if (exist)
			{
				if (tc_strstr(inputparameter, ",1") != NULL)get_iman_specification_pdf_dataset_tag(item_rev_tag, downloadVector);
				if (tc_strstr(inputparameter, ",2") != NULL)get_apqp_pdf_dataset_tag(item_rev_tag,"APQP_Doc", releasestatusList, downloadVector);
			}
			printrecursivebom(children[k], releasestatusList, downloadVector , inputparameter);
		}
	}

	std::string DownloadPDFInterfaceImpl::downloadCOPDF ( const std::string input )
	{
		int ifail = ITK_ok;
		tag_t item_rev_tag = NULL;
		string object_uid = "";
		string user = "";
		string downloadIndex = ""; //
		string preference_file_path = "";
		string returnUid = "";
		tag_t specification_relation_tag = NULLTAG, solution_relation_tag = NULLTAG;
		string temp_folder = getenv("TMP");
		temp_folder = temp_folder + "\\tempFolder";
		//POM_AM__set_application_bypass(true);


		vector<string> inputParameters = split(input, ",");
		vector<tag_t>download_dataset_vector;
				vector<string>releaseStatusVector;
				if (inputParameters.size() < 3)
				{
					ifail = 1;
				}
				else
				{
					char* item_id = NULL, *item_revision_id = NULL;
					object_uid = inputParameters[0];
					user = inputParameters[1];
					ITK__convert_uid_to_tag(object_uid.c_str(), &item_rev_tag);
					ITKCALL(AOM_ask_value_string(item_rev_tag, "item_id", &item_id));
					ITKCALL(AOM_ask_value_string(item_rev_tag, "item_revision_id", &item_revision_id));

					GetPreferenceValue("HASCO_CO_Download_RATE", preference_file_path);
					GetPreferenceValues("HASCO_ASSEMBLE_DOWNLOAD_STATUS", releaseStatusVector);

					int co_solution_objects_count = 0;
					tag_t* co_solution_object_tags = NULL;

					ITKCALL(GRM_find_relation_type(TC_specification_rtype, &specification_relation_tag));
					ITKCALL(GRM_find_relation_type("EC_solution_item_rel", &solution_relation_tag));

					if (tc_strstr(input.c_str(), ",3") != NULL) //下载解决方案零组件下的一般件下的APQP的PDF
					{
						get_apqp_pdf_dataset_tag(item_rev_tag, "EC_solution_item_rel", releaseStatusVector, download_dataset_vector);
					}

					// //查找指定对象下指定关系类型的对象
					ITKCALL(GRM_list_secondary_objects_only(item_rev_tag, solution_relation_tag, &co_solution_objects_count, &co_solution_object_tags));
					if (co_solution_objects_count > 0)
					{
						for (int i = 0; i < co_solution_objects_count; i++) {
							char *object_type = NULL;
							AOM_ask_value_string(co_solution_object_tags[i], "object_type", &object_type);
							printf("object_type=%s\n", object_type);
							if (tc_strcmp(object_type, "PDF") == 0) {
								if (tc_strstr(input.c_str(), ",1") != NULL) //下载解决方案零组件下的PDF
								{
									download_dataset_vector.push_back(co_solution_object_tags[i]);
								}
							}
							else if (tc_strcmp(object_type, "SK2_YBJ Revision") == 0 || tc_strcmp(object_type, "SK2_TYJ Revision") == 0) {
								logical exist = false;
								tag_t ybj_revision_object_tag = co_solution_object_tags[i];
								exist_release_status(ybj_revision_object_tag, releaseStatusVector, &exist); //判断是否存在符合条件的状态
								if (exist)
								{
									if (tc_strstr(input.c_str(), ",2") != NULL) //下载解决方案零组件下的一般件下的PDF
									{
										get_iman_specification_pdf_dataset_tag(ybj_revision_object_tag, download_dataset_vector);
									}
									//if (tc_strstr(input.c_str(), ",3") != NULL) //下载解决方案零组件下的一般件下的APQP的PDF
									//{
									//	get_apqp_pdf_dataset_tag(ybj_revision_object_tag,"APQP_Doc", releaseStatusVector, download_dataset_vector);
									//}
								}
							}
							DOFREE(object_type);
						}
					}

					if(download_dataset_vector.size()>0)returnUid = downloadAndZipPDF(download_dataset_vector,preference_file_path,"CO_Download",item_id,item_revision_id,user);
					DOFREE(co_solution_object_tags);
					DOFREE(item_id);
					DOFREE(item_revision_id);
				}

		//POM_AM__set_application_bypass(false);
		return returnUid;
	}


	std::string DownloadPDFInterfaceImpl::downloadAssemblyPDF ( const std::string input )
	{
		int ifail = ITK_ok;
		tag_t item_rev_tag = NULL;
		string object_uid = "";
		string user = "";
		string downloadIndex = ""; //
		string preference_file_path = "";
		string returnUid = "";
		//POM_AM__set_application_bypass(true);

		vector<string> inputParameters = split(input, ",");
		vector<tag_t>download_dataset_vector;
		if (inputParameters.size() < 3)
		{
			ifail = 1;
		}
		else
		{
			char* item_id = NULL, *item_revision_id = NULL;
			tag_t window = NULLTAG , rule = NULLTAG, top_line = NULLTAG;

			vector<tag_t>download_dataset_vector;
			vector<string>releaseStatusVector;
			object_uid = inputParameters[0];
			user = inputParameters[1];
			ITK__convert_uid_to_tag(object_uid.c_str(), &item_rev_tag);
			ITKCALL(AOM_ask_value_string(item_rev_tag, "item_id", &item_id));
			ITKCALL(AOM_ask_value_string(item_rev_tag, "item_revision_id", &item_revision_id));

			GetPreferenceValue("HASCO_CO_Download_RATE", preference_file_path);
			GetPreferenceValues("HASCO_ASSEMBLE_DOWNLOAD_STATUS", releaseStatusVector);

			if (tc_strstr(input.c_str(), ",1") != NULL)get_iman_specification_pdf_dataset_tag(item_rev_tag, download_dataset_vector);
			if (tc_strstr(input.c_str(), ",2") != NULL)get_apqp_pdf_dataset_tag(item_rev_tag,"APQP_Doc",releaseStatusVector, download_dataset_vector);


			ITKCALL(BOM_create_window(&window));
			ITKCALL(CFM_find("Release Status", &rule)); //Find Revision Rule  //Release Status
			if(rule!=NULLTAG)ITKCALL(BOM_set_window_config_rule(window, rule));//Set Revision Rule

			ITKCALL(BOM_set_window_pack_all(window, true));
			ITKCALL(BOM_set_window_top_line(window, null_tag, item_rev_tag, null_tag, &top_line));
			printrecursivebom(top_line, releaseStatusVector, download_dataset_vector, input.c_str());
			ITKCALL(BOM_close_window(window));

			if(download_dataset_vector.size()>0)returnUid = downloadAndZipPDF(download_dataset_vector,preference_file_path,"Assemble_Download",item_id,item_revision_id,user);
			DOFREE(item_id);
			DOFREE(item_revision_id);
		}

		//POM_AM__set_application_bypass(false);
		return returnUid;
	}
