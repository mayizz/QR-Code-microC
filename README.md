# DMM-Project: Group5

QRCode-Generator
Members: Allaa El-Khodr, Bker Sawalha, Mayiz Zgheib, Lucas Kneffel Otal

Basically, this programm encodes a message received via UART and displays the equivalent QR-Code. 
It auto-detects size and encoding level depending on the input message.
Message must fit the 10-H standards at most given by LCD size.


Dabei ist erstmal der gesamte Algorithmus zu Generierung eines QR-Codes zu implementieren. Weiterhin wollen wir 
mindestens ein Fehler-Level berücksichtigen können. Das bedeutet dass der QR-Code gelesen werden kann, obwohl nicht 
der gesamte Code Sichtbar ist (für den aufnehmenden Sensor).


Allgemeines zum QR-Code


 - Wofür ? - Informationsverschlüsselung

 - Sehr viele verschiedene QR-Code Versionen

 - Deshalb: Auswahl einer QR-Code Version

 - Dadurch wird folgendes festgelegt:

	Größe des QR-Codes
	QR-Code Muster
	maximale Anzahl an Bits der zu codierenden Bitfolge


Orientiert haben wir uns an folgendem Algorithmus (https://blauerbildschirm.wordpress.com/)

Wir haben den Algorithmus in 4 große Bereiche getrennt.

1.Preprocessing

	Dabei wird der eingegebende Text in Binärcode umgewandelt. Zusätzlich wird der Modus, der QR Code Generation bestimmt, sowie die 
	Länge des Codes festgelegt.

	In folgenden Schritten:
	- Numerisch {0001}
	  Alphanumerisch {0010}

	- Übergeben der Länge des zu codierenden Textes 

	- Aufteilen des Textes in Blöcke je nach Modus Wahl
	  Codieren der Buchstaben über Mapping Tabelle in 11bit Wörter

	- Verketten aller entstandenen Bitfolgen


2. Block Trennung

	- Differenz zw. entstandener und max. Anzahl Bitfolge ermitteln
	- Je nach Differenz {0}….{0000} hinzufügen

	- Bitfolge in Bytes (8er Blöcke) aufteilen

	- Letzter Block mit 0 füllen bis 8er Block erreicht

	- Spezielle Blöcke im Wechsel hinzufügen bis max. Anzahl erreicht
	- Spezial1 = {11101100}, Spezial2 = {00010001}


3. Fehlerkorrektur
	
	
	- Nachricht wird in Bytes auf geteilt und in Dezimalzahlen umgewandelt

	- Den Dezimalzahlen entsprechend werden Paritätsbytes durch einen Reed-Solomon-Encoder generiert 

	- Paritätsbytes werden der Nachricht angehängt

	- Vollständige Nachricht inklusive Paritätsbytes wird an den Plotter übermittelt       
 



4. Ausgabe auf dem LCD

	
	- Länge des QR-Codes durch die Version bestimmen, z.B. Version 1 21x21
 
	- Positionserkennungsfelder in jedem QR-Code zur Erkennung der Ausrichtung müssen verbunden sein

	- Somit sind diese Pixels belegt und können nicht überschrieben werden

	- Maske und Fehlerkorrektur Level auswählen, z.B:  Level H, Maske 0
	  32 37 39… = 00100000 00100101 00100111…

		-> (y + x) mod 2 == 0, wenn 0 dann Bit wird geändert
 


 
 

>>>>>>> 131f304d32ab9c102627ca56b81e90229138cd7e
