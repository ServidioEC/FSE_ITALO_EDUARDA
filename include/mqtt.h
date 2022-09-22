#ifndef MQTT_H
#define MQTT_H

void mqtt_start();
void mqtt_client_stop();
void mqtt_client_restart();

void mqtt_envia_mensagem(char *topico, char *mensagem);

#endif