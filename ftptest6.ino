byte doFTP(String requString)
{
	Serial.println(F("SD opened"));

	fclient.connect(fserver, 21);

	if (!eRcv()) return 0;

	fclient.println(F("USER gds25392"));

	if (!eRcv()) return 0;

	fclient.println(F("PASS PPPQiNcL"));

	if (!eRcv()) return 0;

	fclient.println(F("SYST"));

	if (!eRcv()) return 0;

	fclient.println(F("PASV"));

	if (!eRcv()) return 0;

	char *tStr = strtok(outBuf, "(,");
	int array_pasv[6];
	for (int i = 0; i < 6; i++) {
		tStr = strtok(NULL, "(,");
		array_pasv[i] = atoi(tStr);
		if (tStr == NULL)
		{
			Serial.println(F("Bad PASV Answer"));
		}
	}

	unsigned int hiPort, loPort;

	hiPort = array_pasv[4] << 8;
	loPort = array_pasv[5] & 255;

	Serial.print(F("Data port: "));
	hiPort = hiPort | loPort;
	Serial.println(hiPort);

	if (dclient.connect(fserver, hiPort)) {
		Serial.println(F("Data connected"));
	}
	else {
		Serial.println(F("Data connection failed"));
		fclient.stop();

		return 0;
	}

#ifdef FTPWRITE
	fclient.print(F("STOR "));
	fclient.println(fileName);
#else
	fclient.println("CWD /gds/gds/specials/forecasts/tables/germany");
	fclient.print("RETR ");
	fclient.println(requString);
	i = 0;
#endif

	if (!eRcv())
	{
		dclient.stop();
		return 0;
	}

#ifdef FTPWRITE
	Serial.println(F("Writing"));

	byte clientBuf[64];
	int clientCount = 0;

	while (fh.available())
	{
		clientBuf[clientCount] = fh.read();
		clientCount++;

		if (clientCount > 63)
		{
			dclient.write(clientBuf, 64);
			clientCount = 0;
		}
	}

	if (clientCount > 0) dclient.write(clientBuf, clientCount);

#else
	while (dclient.connected())
	{
		mischen();
		while (dclient.available())
		{
			mischen();
			c = dclient.read();
			if (i == 16)
			{
				//Serial.write(c);
				//Serial.print(c);
				fchars.concat(c);
				if (c == 10)
				{
					fchars = fchars.substring(17, 20);
					if (requString == "Daten_Deutschland_morgen_frueh"){
						//Serial.println(fchars);
						tcastfrueh = fchars.toInt();
						fchars = "";
						Serial.println(tcastfrueh);
					}
					else
					{//Serial.println(fchars);
						tcastspaet = fchars.toInt();
						fchars = "";
						Serial.println(tcastspaet);
					}
				}
			}
			if (c == 10)
			{
				i++;
			}

			//fh.write(c);
			//Serial.write(c);

			//delay(10);
			//fchars.concat(10);
			//Serial.println(fchars);
		}

		//
	}
#endif

	dclient.stop();
	Serial.println(F("Data disconnected"));

	if (!eRcv()) return 0;

	fclient.println(F("QUIT"));

	if (!eRcv()) return 0;

	fclient.stop();
	Serial.println(F("Command disconnected"));

	Serial.println(F("SD closed"));
	return 1;
}

byte eRcv()
{
	byte respCode;
	byte thisByte;

	while (!fclient.available()) delay(1);

	respCode = fclient.peek();
	Serial.write(respCode);
	outCount = 0;

	while (fclient.available())
	{
		thisByte = fclient.read();

		Serial.write(thisByte);

		if (outCount < 127)
		{
			outBuf[outCount] = thisByte;
			outCount++;
			outBuf[outCount] = 0;
		}
	}

	if (respCode >= '4')
	{
		efail();
		return 0;
	}

	return 1;
}

void efail()
{
	byte thisByte = 0;

	fclient.println(F("QUIT"));

	while (!fclient.available()) delay(1);

	while (fclient.available())
	{
		thisByte = fclient.read();
		Serial.write(thisByte);
	}

	fclient.stop();
	Serial.println(F("Command disconnected"));

	Serial.println(F("SD closed"));
}