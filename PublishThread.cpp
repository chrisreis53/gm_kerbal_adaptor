/*
 * Copyright 2007-2016 United States Government as represented by the
 * Administrator of The National Aeronautics and Space Administration.
 * No copyright is claimed in the United States under Title 17, U.S. Code.
 * All Rights Reserved.
 */


/**
 * @file PublishThread.cpp
 *
 * This file contains a simple example publisher using the GMSEC standard C++ API.
 *
 */

#include "PublishThread.hpp"

#include <string>
#include <curl/curl.h>

#include "json.hpp"


using json = nlohmann::json;
using namespace gmsec::api;

json dictionary;
std::string url = "http://192.168.1.4:8085/telemachus/datalink?";
std::string v_url;
std::vector<std::string> v_meas;
std::string o_url;
std::vector<std::string> o_meas;

static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

PublishThread::PublishThread(Config &c)
	: config(c),
	  connection(0)
{
	example::initialize(config);
}


PublishThread::~PublishThread()
{
	teardown();
}


void CALL_TYPE PublishThread::run()
{
	try
	{
		setup();

		//o Output GMSEC API and middleware version information
		GMSEC_INFO << Connection::getAPIVersion();
		GMSEC_INFO << connection->getLibraryVersion();

		//o Delay to allow subscriber thread to set up
		example::millisleep(100);
    std::ifstream myfile ("dictionary.json");
    std::string str((std::istreambuf_iterator<char>(myfile)), (std::istreambuf_iterator<char>()));
    dictionary = json::parse(str);
    for (size_t i = 0; i < dictionary["subsystems"].size(); i++) {
      if(dictionary["subsystems"][i]["name"] == "Vehicle"){
        v_url += url;
        for (size_t j = 0; j < dictionary["subsystems"][i]["measurements"].size(); j++) {
          std::string id(dictionary["subsystems"][i]["measurements"][j]["identifier"].get<std::string>());
          v_url += id + "=" + id + "&";
        }
      }
    }
    std::cout << v_url << '\n';
		while (true) {
			get_v();
			//get_o();
			//publish("GMSEC.TEST.PUBLISH");
			example::millisleep(250);
		}

		teardown();
	}
	catch (Exception& e)
	{
		GMSEC_ERROR << "Failure: " << e.what();
	}
}


void PublishThread::publish(const char* subject)
{
	//o Create message
	Message message(subject, Message::PUBLISH);

	//o Add fields
	int i = 123;

	message.addField("CHAR-FIELD", (GMSEC_CHAR) 'c');
	message.addField("BOOL-FIELD", false);
	message.addField("I32-FIELD", (GMSEC_I32) i);
	message.addField("U16-FIELD", (GMSEC_U16) i);
	message.addField("STRING-FIELD", "This is a test");
	message.addField("F64-FIELD", GMSEC_F64(123 / .1));
	message.addField("BIN-FIELD", (GMSEC_BIN) "JLMNOPQ", 7);

	//o Publish Message
	connection->publish(message);

	//o Display XML representation of message
	GMSEC_INFO << "Published:\n" << message.toXML();
}

// void PublishThread::get_test(){
//   std::string filename = "dictionary.json";
//   FILE* pFile = fopen(fileName.c_str(), "rb");
//   char buffer[65536];
//   rapidjson::FileReadStream is(pFile, buffer, sizeof(buffer));
//   rapidjson::Document document;
//   document.rapidjson::ParseStream(is);
//
// }

void PublishThread::get_v(){
	CURL *curl;
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, v_url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		json js = json::parse(readBuffer);
		//std::cout << readBuffer << std::endl;
    std::string subject = "GMSEC.KSP.MSG.TLM.PROCESSED.V";
    Message message(subject.c_str(), Message::PUBLISH);
    for (json::iterator it = js.begin(); it != js.end(); ++it) {
      if (it.value().is_string()) {
        //std::cout << it.key() << " : " << it.value().get<std::string>() << "\n";
        std::string field(it.key());
        message.addField(field.c_str(), it.value().get<std::string>().c_str());
      }
      if (it.value().is_number()) {
        //std::cout << it.key() << " : " << it.value().get<float>() << "\n";
        std::string field(it.key());
        message.addField(field.c_str(), it.value().get<float>());
      }
      //message.addField(std::string(it.key()).c_str(), std::string(it.value()).c_str());
    }
    // for (size_t i = 0; i < js.size(); i++) {
    //   std::cout << js[i].get<std::string>() << '\n';
    // }
    connection->publish(message);
		// float msnTime = js.at("v.missionTime");
		// long alt = js.at("v.altitude");
		// float latitude = js.at("v.lat");
		// float longitude = js.at("v.long");
		// std::string name = js.at("v.name");
		// std::string subject = "GMSEC.KSP.MSG.TLM.PROCESSED.V";
		// Message message(subject.c_str(), Message::PUBLISH);
		// message.addField("ALTITUDE", (GMSEC_U32) alt);
		// message.addField("MISSION-TIME", msnTime);
		// message.addField("LONGITUDE", longitude);
		// message.addField("LATITUDE", latitude);
		// connection->publish(message);
		// std::cout << msnTime << " " << alt << std::endl;
	}
}

void PublishThread::get_o(){
	std::string url = "http://192.168.1.4:8085/telemachus/datalink?";
	url += "name=v.name&";
	url += "orbit=o.orbitPatches";
	CURL *curl;
	CURLcode res;
	std::string readBuffer;
	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
		res = curl_easy_perform(curl);
		curl_easy_cleanup(curl);
		json js = json::parse(readBuffer);
		std::cout << readBuffer << std::endl;
		json jo = js["orbit"][0];
		//std::cout << jo.dump() << std::endl;
		//std::string name = js.at("name");
		float inclination = jo.at("inclination");
		std::string subject = "GMSEC.KSP.MSG.TLM.PROCESSED.O";
		Message message(subject.c_str(), Message::PUBLISH);
		message.addField("INCLINATION", inclination);
		connection->publish(message);
	}
}

void PublishThread::setup()
{
	//o Create the Connection
	connection = Connection::create(config);

	//o Connect to the GMSEC bus
	connection->connect();
}


void PublishThread::teardown()
{
	if (connection)
	{
		connection->disconnect();
		Connection::destroy(connection);
	}
}
