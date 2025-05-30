/*
 * This file is part of libemqtt.
 *
 * libemqtt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libemqtt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libemqtt.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 *
 * Created by Filipe Varela on 09/10/16.
 * Copyright 2009 Caixa Mágica Software. All rights reserved.
 *
 * Fork developed by Vicente Ruiz Rodríguez
 * Copyright 2012 Vicente Ruiz Rodríguez <vruiz2.0@gmail.com>. All rights reserved.
 *
 */

#include <string.h>
#include <libemqtt.h>
#include "luat_base.h"
#include "luat_mem.h"

#define MQTT_DUP_FLAG     (1<<3)
#define MQTT_QOS0_FLAG    (0<<1)
#define MQTT_QOS1_FLAG    (1<<1)
#define MQTT_QOS2_FLAG    (2<<1)

#define MQTT_RETAIN_FLAG  1

#define MQTT_CLEAN_SESSION  (1<<1)
#define MQTT_WILL_FLAG      (1<<2)
#define MQTT_WILL_RETAIN    (1<<5)
#define MQTT_USERNAME_FLAG  (1<<7)
#define MQTT_PASSWORD_FLAG  (1<<6)

#define LUAT_LOG_TAG "mqtt"
#include "luat_log.h"


uint8_t mqtt_num_rem_len_bytes(const uint8_t* buf) {
	uint8_t num_bytes = 1;
	//printf("mqtt_num_rem_len_bytes\n");
	if ((buf[1] & 0x80) == 0x80) {
		num_bytes++;
		if ((buf[2] & 0x80) == 0x80) {
			num_bytes ++;
			if ((buf[3] & 0x80) == 0x80) {
				num_bytes ++;
			}
		}
	}
	return num_bytes;
}

uint32_t mqtt_parse_rem_len(const uint8_t* buf) {
	uint32_t multiplier = 1;
	uint32_t value = 0;
	uint8_t digit;
	//printf("mqtt_parse_rem_len\n");
	buf++;	// skip "flags" byte in fixed header
	do {
		digit = *buf;
		value += (digit & 127) * multiplier;
		multiplier *= 128;
		buf++;
	} while ((digit & 128) != 0);
	return value;
}

uint16_t mqtt_parse_msg_id(const uint8_t* buf) {
	uint8_t type = MQTTParseMessageType(buf);
	uint8_t qos = MQTTParseMessageQos(buf);
	uint16_t id = 0;
	//printf("mqtt_parse_msg_id\n");
	if(type >= MQTT_MSG_PUBLISH && type <= MQTT_MSG_UNSUBACK) {
		if(type == MQTT_MSG_PUBLISH) {
			if(qos != 0) {
				// fixed header length + Topic (UTF encoded)
				// = 1 for "flags" byte + rlb for length bytes + topic size
				uint8_t rlb = mqtt_num_rem_len_bytes(buf);
				uint8_t offset = *(buf+1+rlb)<<8;	// topic UTF MSB
				offset |= *(buf+1+rlb+1);			// topic UTF LSB
				offset += (1+rlb+2);					// fixed header + topic size
				id = *(buf+offset)<<8;				// id MSB
				id |= *(buf+offset+1);				// id LSB
			}
		} else {
			// fixed header length
			// 1 for "flags" byte + rlb for length bytes
			uint8_t rlb = mqtt_num_rem_len_bytes(buf);
			id = *(buf+1+rlb)<<8;	// id MSB
			id |= *(buf+1+rlb+1);	// id LSB
		}
	}
	return id;
}

uint16_t mqtt_parse_pub_topic(const uint8_t* buf, uint8_t* topic) {
	const uint8_t* ptr;
	uint16_t topic_len = mqtt_parse_pub_topic_ptr(buf, &ptr);
	//printf("mqtt_parse_pub_topic\n");
	if(topic_len != 0 && ptr != NULL) {
		memcpy(topic, ptr, topic_len);
	}
	return topic_len;
}

uint16_t mqtt_parse_pub_topic_ptr(const uint8_t* buf, const uint8_t **topic_ptr) {
	uint16_t len = 0;
	//printf("mqtt_parse_pub_topic_ptr\n");
	if(MQTTParseMessageType(buf) == MQTT_MSG_PUBLISH) {
		// fixed header length = 1 for "flags" byte + rlb for length bytes
		uint8_t rlb = mqtt_num_rem_len_bytes(buf);
		len = *(buf+1+rlb)<<8;	// MSB of topic UTF
		len |= *(buf+1+rlb+1);	// LSB of topic UTF
		// start of topic = add 1 for "flags", rlb for remaining length, 2 for UTF
		*topic_ptr = (buf + (1+rlb+2));
	} else {
		*topic_ptr = NULL;
	}
	return len;
}

uint32_t mqtt_parse_publish_msg(const uint8_t* buf, uint8_t* msg) {
	const uint8_t* ptr;
	//printf("mqtt_parse_publish_msg\n");
	uint32_t msg_len = mqtt_parse_pub_msg_ptr(buf, &ptr);
	if(msg_len != 0 && ptr != NULL) {
		memcpy(msg, ptr, msg_len);
	}
	return msg_len;
}

uint32_t mqtt_parse_pub_msg_ptr(const uint8_t* buf, const uint8_t **msg_ptr) {
	uint32_t len = 0;
	//printf("mqtt_parse_pub_msg_ptr\n");
	if(MQTTParseMessageType(buf) == MQTT_MSG_PUBLISH) {
		// message starts at
		// fixed header length + Topic (UTF encoded) + msg id (if QoS>0)
		uint8_t rlb = mqtt_num_rem_len_bytes(buf);
		uint8_t offset = (*(buf+1+rlb))<<8;	// topic UTF MSB
		offset |= *(buf+1+rlb+1);			// topic UTF LSB
		offset += (1+rlb+2);				// fixed header + topic size
		if(MQTTParseMessageQos(buf)) {
			offset += 2;					// add two bytes of msg id
		}
		*msg_ptr = (buf + offset);
				
		// offset is now pointing to start of message
		// length of the message is remaining length - variable header
		// variable header is offset - fixed header
		// fixed header is 1 + rlb
		// so, lom = remlen - (offset - (1+rlb))
      	len = mqtt_parse_rem_len(buf) - (offset-(rlb+1));
	} else {
		*msg_ptr = NULL;
	}
	return len;
}

void mqtt_init(mqtt_broker_handle_t* broker, const char* clientid) {
	// Connection options
	broker->alive = 300; // 300 seconds = 5 minutes
	broker->seq = 1; // Sequency for message indetifiers
	// Client options
	memset(broker->clientid, 0, sizeof(broker->clientid));
	memset(broker->username, 0, sizeof(broker->username));
	memset(broker->password, 0, sizeof(broker->password));
	if(clientid) {
		memcpy(broker->clientid, clientid, strlen(clientid));
	}
	// Will topic
	broker->clean_session = 1;
}

void mqtt_init_auth(mqtt_broker_handle_t* broker, const char* username, const char* password) {
	if(username && username[0] != '\0')
		memcpy(broker->username, username, strlen(username)+1);
	if(password && password[0] != '\0')
		memcpy(broker->password, password, strlen(password)+1);
}

void mqtt_set_alive(mqtt_broker_handle_t* broker, uint16_t alive) {
	broker->alive = alive;
}

int mqtt_connect(mqtt_broker_handle_t* broker)
{
	uint8_t flags = 0x00;

	uint16_t clientidlen = strlen(broker->clientid);
	uint16_t usernamelen = strlen(broker->username);
	uint16_t passwordlen = strlen(broker->password);
	uint16_t payload_len = clientidlen + 2;

	// Preparing the flags
	if(usernamelen) {
		payload_len += usernamelen + 2;
		flags |= MQTT_USERNAME_FLAG;
	}
	if(passwordlen) {
		payload_len += passwordlen + 2;
		flags |= MQTT_PASSWORD_FLAG;
	}
	if(broker->clean_session) {
		flags |= MQTT_CLEAN_SESSION;
	}
	if (broker->will_len > 0) {
		payload_len += broker->will_len;
		flags |= MQTT_WILL_FLAG;
		if (broker->will_retain)
			flags |= MQTT_WILL_RETAIN;
		if (broker->will_qos) {
			flags |= (broker->will_qos << 3);
		}
	}

	// Variable header
	uint8_t var_header[] = {
		0x00,0x04,0x4d,0x51,0x54,0x54, // Protocol name: MQTT
		0x04, // Protocol version 3.1.1
		flags, // Connect flags
		broker->alive>>8, broker->alive&0xFF, // Keep alive
	};


   	// Fixed header
    // uint16_t fixedHeaderSize = 2;    // Default size = one byte Message Type + one byte Remaining Length
    uint16_t remainLen = sizeof(var_header)+payload_len;
    uint8_t buf[4] = {0};
	int rc = 0;
	uint32_t length = remainLen;
	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	size_t fixed_header_len = rc + 1;

    // Allocate memory for the fixed header (3 bytes
    uint8_t fixed_header[8] = {0};
    
    // Message Type
    fixed_header[0] = MQTT_MSG_CONNECT;

    // Remaining Length
	memcpy(fixed_header + 1, buf, fixed_header_len - 1);

	uint16_t offset = 0;
	uint32_t packet_size = fixed_header_len+sizeof(var_header)+payload_len;
	uint8_t *packet = luat_heap_malloc(packet_size);
	if (packet == NULL) {
		LLOGE("out of memory when malloc connect packet");
		return -2;
	}
	memset(packet, 0, packet_size);
	memcpy(packet, fixed_header, fixed_header_len);
	offset += fixed_header_len;
	memcpy(packet+offset, var_header, sizeof(var_header));
	offset += sizeof(var_header);
	// Client ID - UTF encoded
	packet[offset++] = clientidlen>>8;
	packet[offset++] = clientidlen&0xFF;
	if (clientidlen)
		memcpy(packet+offset, broker->clientid, clientidlen);
	offset += clientidlen;

	
	if (broker->will_len) {
		memcpy(packet+offset, broker->will_data, broker->will_len);
		offset += broker->will_len;
	}

	if(usernamelen) {
		// Username - UTF encoded
		packet[offset++] = usernamelen>>8;
		packet[offset++] = usernamelen&0xFF;
		memcpy(packet+offset, broker->username, usernamelen);
		offset += usernamelen;
	}

	if(passwordlen) {
		// Password - UTF encoded
		packet[offset++] = passwordlen>>8;
		packet[offset++] = passwordlen&0xFF;
		memcpy(packet+offset, broker->password, passwordlen);
		offset += passwordlen;
	}

	// Send the packet
	if(broker->send(broker->socket_info, packet, packet_size) < packet_size) {
		luat_heap_free(packet);
		return -1;
	}
	luat_heap_free(packet);
	return 1;
}

int mqtt_disconnect(mqtt_broker_handle_t* broker) {
	static const uint8_t packet[] = {
		MQTT_MSG_DISCONNECT, // Message Type, DUP flag, QoS level, Retain
		0x00 // Remaining length
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_ping(mqtt_broker_handle_t* broker) {
	static const uint8_t packet[] = {
		MQTT_MSG_PINGREQ, // Message Type, DUP flag, QoS level, Retain
		0x00 // Remaining length
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_publish(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint32_t msg_len, uint8_t retain) {
	return mqtt_publish_with_qos(broker, topic, msg, msg_len, retain, 0, NULL);
}

int mqtt_publish_with_qos(mqtt_broker_handle_t* broker, const char* topic, const char* msg, uint32_t msg_len, uint8_t retain, uint8_t qos, uint16_t* message_id) {
	uint8_t small_head[64];
	uint8_t small_packet[128];
	uint16_t topiclen = strlen(topic);
	uint32_t msglen = msg_len;
	// uint32_t tem_len;
	uint8_t qos_flag = MQTT_QOS0_FLAG;
	uint8_t qos_size = 0; // No QoS included
	void *p1 = NULL;
	void *p2 = NULL;
	uint8_t *var_header;
	uint8_t *packet;
	if(qos == 1) {
		qos_size = 2; // 2 bytes for QoS
		qos_flag = MQTT_QOS1_FLAG;
	}
	else if(qos == 2) {
		qos_size = 2; // 2 bytes for QoS
		qos_flag = MQTT_QOS2_FLAG;
	}

	// Variable header
	size_t var_header_len = topiclen+2+qos_size;
	if (var_header_len <= sizeof(small_head)) {
		var_header = small_head;
	} else {
		p1 = luat_heap_malloc(var_header_len);
		if (p1 == NULL) {
			LLOGE("out of memory when malloc var head");
			goto ERROR_OUT;
		}
		var_header = (uint8_t *)p1;
	}

	memset(var_header, 0, var_header_len);
	var_header[0] = topiclen>>8;
	var_header[1] = topiclen&0xFF;
	memcpy(var_header+2, topic, topiclen);
	if(qos_size) {
		var_header[topiclen+2] = broker->seq>>8;
		var_header[topiclen+3] = broker->seq&0xFF;
		if(message_id) { // Returning message id
			*message_id = broker->seq;
		}
		broker->seq++;
	}

	// Fixed header
	uint32_t remainLen = var_header_len+msglen;
	uint8_t buf[4] = {0};
	int rc = 0;
	uint32_t length = remainLen;
	do
	{
		char d = length % 128;
		length /= 128;
		/* if there are more digits to encode, set the top bit of this digit */
		if (length > 0)
			d |= 0x80;
		buf[rc++] = d;
	} while (length > 0);
	size_t fixed_header_len = rc + 1;
	uint8_t fixed_header[8];
    
   	// Message Type, DUP flag, QoS level, Retain
   	fixed_header[0] = MQTT_MSG_PUBLISH | qos_flag;
	if(retain) {
		fixed_header[0] |= MQTT_RETAIN_FLAG;
   	}
	memcpy(fixed_header+1, buf, rc);

	uint8_t header_size = fixed_header_len+var_header_len;
	uint32_t total_size = header_size + msg_len;
	int ret = 0;
	if (total_size <= sizeof(small_packet)) {
		packet = small_packet;
	} else {
#ifdef __LUATOS__
		if (total_size > 8192) {
#else
		if (total_size > 16384) {
#endif
			p2 = luat_heap_opt_malloc(LUAT_HEAP_AUTO,total_size);
		} else {
			p2 = luat_heap_opt_malloc(LUAT_HEAP_SRAM,total_size);
		}
		if (p2 == NULL) {
			LLOGE("out of memory when malloc publish packet");
			goto ERROR_OUT;
		}
		packet = (uint8_t *)p2;
	}

	memset(packet, 0, header_size);
	memcpy(packet, fixed_header, fixed_header_len);
	memcpy(packet+fixed_header_len, var_header, var_header_len);

	// Send the packet
	memcpy(packet + header_size, msg, msg_len);
	ret = broker->send(broker->socket_info, packet, total_size);
	if(ret < 0 || ret < total_size) {
		goto ERROR_OUT;
	}
	if (p1) luat_heap_free(p1);
	if (p2) luat_heap_free(p2);
	return 1;
ERROR_OUT:
	if (p1) luat_heap_free(p1);
	if (p2) luat_heap_free(p2);
	return -1;
}

int mqtt_pubrel(mqtt_broker_handle_t* broker, uint16_t message_id) {
	uint8_t packet[] = {
		MQTT_MSG_PUBREL | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		0x02, // Remaining length
		message_id>>8,
		message_id&0xFF
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_puback(mqtt_broker_handle_t* broker, uint16_t message_id) {
	uint8_t packet[] = {
		MQTT_MSG_PUBACK | MQTT_QOS0_FLAG, // Message Type, DUP flag, QoS level, Retain
		0x02, // Remaining length
		message_id>>8,
		message_id&0xFF
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_pubrec(mqtt_broker_handle_t* broker, uint16_t message_id) {
	uint8_t packet[] = {
		MQTT_MSG_PUBREC | MQTT_QOS0_FLAG, // Message Type, DUP flag, QoS level, Retain
		0x02, // Remaining length
		message_id>>8,
		message_id&0xFF
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}



int mqtt_pubcomp(mqtt_broker_handle_t* broker, uint16_t message_id) {
	uint8_t packet[] = {
		MQTT_MSG_PUBCOMP | MQTT_QOS0_FLAG, // Message Type, DUP flag, QoS level, Retain
		0x02, // Remaining length
		message_id>>8,
		message_id&0xFF
	};

	// Send the packet
	if(broker->send(broker->socket_info, packet, sizeof(packet)) < sizeof(packet)) {
		return -1;
	}

	return 1;
}

int mqtt_subscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id, uint8_t qos) {
	uint16_t topiclen = strlen(topic);
	if (qos>2) qos=0;
	
	// Variable header
	uint8_t var_header[2]; // Message ID
	var_header[0] = broker->seq>>8;
	var_header[1] = broker->seq&0xFF;
	if(message_id) { // Returning message id
		*message_id = broker->seq;
	}
	broker->seq++;

	// utf topic
	size_t utf_topic_len = topiclen + 3;
	uint8_t *utf_topic = luat_heap_malloc(utf_topic_len); // Topic size (2 bytes), utf-encoded topic, QoS byte
	if (utf_topic == NULL) {
		LLOGE("out of memory when malloc subscribe utf_topic");
		return -1;
	}
	memset(utf_topic, 0, utf_topic_len);
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);
	utf_topic[topiclen+2] |= qos;

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_SUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+utf_topic_len
	};
	size_t packet_len = sizeof(var_header)+sizeof(fixed_header)+utf_topic_len;
	uint8_t *packet = luat_heap_malloc(packet_len);
	if (packet == NULL) {
		LLOGE("out of memory when malloc subscribe packet");
		return -1;
	}
	memset(packet, 0, packet_len);
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, utf_topic_len);

	// Send the packet
	if(broker->send(broker->socket_info, packet, packet_len) < packet_len) {
		luat_heap_free(utf_topic);
		luat_heap_free(packet);
		return -1;
	}
	luat_heap_free(utf_topic);
	luat_heap_free(packet);
	return 1;
}

int mqtt_unsubscribe(mqtt_broker_handle_t* broker, const char* topic, uint16_t* message_id) {
	uint16_t topiclen = strlen(topic);

	// Variable header
	uint8_t var_header[2]; // Message ID
	var_header[0] = broker->seq>>8;
	var_header[1] = broker->seq&0xFF;
	if(message_id) { // Returning message id
		*message_id = broker->seq;
	}
	broker->seq++;

	// utf topic
	size_t utf_topic_len = topiclen + 2;
	uint8_t *utf_topic = luat_heap_malloc(utf_topic_len); // Topic size (2 bytes), utf-encoded topic
	if (utf_topic == NULL) {
		LLOGE("out of memory when malloc subscribe utf_topic");
		return -1;
	}
	memset(utf_topic, 0, utf_topic_len);
	utf_topic[0] = topiclen>>8;
	utf_topic[1] = topiclen&0xFF;
	memcpy(utf_topic+2, topic, topiclen);

	// Fixed header
	uint8_t fixed_header[] = {
		MQTT_MSG_UNSUBSCRIBE | MQTT_QOS1_FLAG, // Message Type, DUP flag, QoS level, Retain
		sizeof(var_header)+utf_topic_len
	};
	size_t packet_len = sizeof(var_header)+sizeof(fixed_header)+utf_topic_len;
	uint8_t packet[1024];
	memset(packet, 0, packet_len);
	memcpy(packet, fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, utf_topic_len);

	// Send the packet
	if(broker->send(broker->socket_info, packet, packet_len) < packet_len) {
		luat_heap_free(utf_topic);
		return -1;
	}
	luat_heap_free(utf_topic);
	return 1;
}

int mqtt_set_will(mqtt_broker_handle_t* broker, const char* topic, 
						const char* payload, size_t payload_len, 
						uint8_t qos, size_t retain) {
	if (broker == NULL)
		return -1;
	//LLOGD("will %s %.*s %d %d", topic, payload_len, payload, qos, retain);
	// 如果之前有数据, 那就释放掉
	if (broker->will_data != NULL) {
		broker->will_len = 0;
		luat_heap_free(broker->will_data);
		broker->will_data = NULL;
	}
	if (topic == NULL) {
		LLOGI("will topic is NULL");
		return 0;
	}
	size_t topic_len = strlen(topic);
	broker->will_data = luat_heap_malloc(topic_len + 2 + payload_len + 2);
	if (broker->will_data == NULL) {
		return -2;
	}
	broker->will_data[0] = (uint8_t)(topic_len >> 8);
	broker->will_data[1] = (uint8_t)(topic_len & 0xFF);
	memcpy(broker->will_data + 2, topic, topic_len);
	
	broker->will_data[2  + topic_len] = (uint8_t)(payload_len >> 8);
	broker->will_data[2  + topic_len + 1] = (uint8_t)(payload_len & 0xFF);
	if (payload_len)
		memcpy(broker->will_data + 2 + topic_len + 2, payload, payload_len);

	broker->will_qos = qos > 2 ? 0 : qos;
	broker->will_retain = retain;
	broker->will_len = topic_len + 2 + payload_len + 2;
	//LLOGD("will len %d", broker->will_len);
	return 0;
}

