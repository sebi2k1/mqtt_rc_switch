/**
 * MIT License
 *
 * Copyright (c) 2016 Sebastian Haas (dev@sebastianhaas.info)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <string>
#include <iostream>
#include <sstream>

#include <stdio.h>
#include <unistd.h>

#include <mosquittopp.h>

#include <RCSwitch.h>
template <typename T>

T lexical_cast(const std::string& str)
{
    T var;
    std::istringstream iss;
    iss.str(str);
    iss >> var;
    // deal with any error bits that may have been set on the stream
    return var;
}

class MqttRcSwitch : public mosqpp::mosquittopp
{
	public:
		MqttRcSwitch(const char *id, const char *host, int port)
      : mosquittopp(id)
    {
      if (wiringPiSetup() == -1)
      {
        printf("Failed to initialzie wiringPi\n");
        return;
      }

      m_RcSwitch.enableTransmit(0);

      connect(host, port, 60 /* keep alive */);
    }

		~MqttRcSwitch() {}

		void on_connect(int rc)
    {
      printf("Connected with code %d.\n", rc);

      if (rc == 0)
    		subscribe(NULL, "/rcswitch/+");
    }

		void on_message(const struct mosquitto_message *message)
    {
      std::string topic(message->topic);
      std::string pl((const char *)message->payload, message->payloadlen);

      // Extract values from string in form XXXXX:C:V
      std::string sys = topic.substr(10, 5);
      int channel = lexical_cast<int>(topic.substr(16, 1));

      int value = lexical_cast<int>(pl);

      // Get realtime prio
      piHiPri(20);

      if (value)
        m_RcSwitch.switchOn(sys.c_str(), channel);
      else
        m_RcSwitch.switchOff(sys.c_str(), channel);

      // TODO: Restore default prio
    }

		void on_subscribe(int mid, int qos_count, const int *granted_qos) { }

  private:
    RCSwitch m_RcSwitch;
};

int main(int argc, char ** argv)
{
  daemon(0, 0);

  // Create MQTT RC Switch
  MqttRcSwitch rcSwitch("rcswitch", "localhost", 1883);

  while(1)
  {
    int rc = rcSwitch.loop();

    if (rc)
      rcSwitch.reconnect();
  }

  return 0;
}
