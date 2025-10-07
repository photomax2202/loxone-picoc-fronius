// INPUT
// I1 - Entladeschutz Niedertarif	>	Entladen Sperren
// I2 - Entladeschutz Wallbox		>	Entladen mit Definierter Leistung I9 Eigenverbrauch
// I3 - Ausgleichsladung				>	Laden mit Definierter Leistung I8 PV Produktion oder 500 W wenn I8 unter 100 W
// I4 - StorCtl_Mod INPUT
// I5 - InWRte INPUT
// I6 - OutWRte INPUT
// I8 - PV Produktion
// I9 - Eigenverbrauch
// I10 - Stromtarif_NT
// I11 - WChaMax
//
// O1 - StorCtl_Mod
// O2 - InWRte
// O3 - OutWRte
//
// Modi
// Entlden Sperren (O1=2, O3=0%)
// Laden Erzwingen mit definierter Leistung (O1 =3,O3/-O2=Val)
// Entladen Erzwingen mit definierter Leistung (O1 =3,O2/-O3=Val)

//Kostanten
#define BUFF_SIZE 256
char szBuffer[BUFF_SIZE];
int localStorCtl_Mod;
float localInWRte, localOutWRte;
// Funktionen
float getChargeValuePos(float aValue)
{
float lWChaMax;
lWChaMax = getinput(10);
	if (lWChaMax > 0)
	{
	 	return 100/lWChaMax*aValue;
	} else {
		return 0;
	 }
}

float getChargeValueNeg(float aValue)
{
	return getChargeValuePos(aValue) * -1;
}

void resetChargeControl(int i)
{
	setoutput(0,0);
	setoutput(1,0);
	setoutput(2,0);
	setoutputtext(0,"Keine Ladesteuerung");
}

void setBlockUncharge(int i)
{
	setoutput(0,2);
	setoutput(1,0);
	setoutput(2,0);
	setoutputtext(0,"Entladen Gesperrt");
}

void setChargeValue(float aValue)
{
	setoutput(0,3);
	setoutput(1,getChargeValuePos(aValue));
	setoutput(2,getChargeValueNeg(aValue));
	szBuffer = "";
	sprintf(szBuffer,"Laden mit definierter Leistung: %f W",aValue);
	setoutputtext(0,szBuffer);
}

void setDischargeValue(float aValue)
{
	setoutput(0,3);
	setoutput(1,getChargeValueNeg(aValue));
	setoutput(2,getChargeValuePos(aValue));
	szBuffer = "";
	sprintf(szBuffer,"Entladen mit definierter Leistung: %f W",aValue);
	setoutputtext(0,szBuffer);
}
// Main Program
while(TRUE)
{
int lActive;
lActive = getinput(12);
//if (lActive > 0)
//{
	float lUnchargeProtectNt, lUnchargeProtectWb, lDifferenceCharge, lOwnUsage, lPvProduction;
	lUnchargeProtectNt = getinput(0);
	lUnchargeProtectWb = getinput(1);
	lDifferenceCharge = getinput(2);
	lOwnUsage = getinput(8)*1000;
	lPvProduction = getinput(7)*1000;

	if (lUnchargeProtectNt > 0)
	{
		setBlockUncharge(0);
	}	else {
		if (lUnchargeProtectWb > 0)
		{
			setDischargeValue(lOwnUsage);
		} else {
			if (lDifferenceCharge > 0)
			{
				if (lPvProduction > 100)
				{
					setChargeValue(lPvProduction-100);
				} else {
					setChargeValue(500);
				}	
			} else {
				resetChargeControl(0);
			}
		}
	}
//} else {
//	resetChargeControl(0);
//}
}
