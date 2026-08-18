#pragma once

#include <IPAddress.h>

// Joins the network named in secrets.h. Returns false rather than blocking
// forever, so the caller decides what a failed join means. On failure it logs
// every network the radio could see, which distinguishes "SSID not found" from
// "password rejected" - they look identical otherwise.
bool networkConnect();

bool networkIsUp();
IPAddress networkIp();
int networkRssi();
