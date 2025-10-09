/*
INPUT
I1 - Entladeschutz Niedertarif	>	Entladen Sperren
I2 - Entladeschutz Wallbox		>	Entladen mit definierter Leistung I9 Eigenverbrauch
I3 - Ausgleichsladung				>	Laden mit definierter Leistung I8 PV Produktion - 100 W oder 500 W bei NT
I4 - StorCtl_Mod INPUT
I5 - InWRte INPUT
I6 - OutWRte INPUT
I8 - PV Produktion
I9 - Eigenverbrauch
I10 - Stromtarif_NT
I11 - WChaMax
I12 - SOC
//
O1 - StorCtl_Mod
O2 - InWRte
O3 - OutWRte
//
Modi
Entlden Sperren (O1=2, O3=0%)
Laden Erzwingen mit definierter Leistung (O1 =3,O3/-O2=Val)
Entladen Erzwingen mit definierter Leistung (O1 =3,O2/-O3=Val)
*/

//Kostanten
#define BUFF_SIZE 256
#define SOC_TH_RECHARGE_LOW 9.5
#define SOC_TH_RECHARGE_HIGH 10
#define STATE_OUTPUT_INTERVAL 600
#define FUNCTION_INDEX_RESET 0
#define FUNCTION_INDEX_CHARGE 1
#define FUNCTION_INDEX_DISCHARGE 2
#define FUNCTION_INDEX_BLOCK 3
char gBuffer[BUFF_SIZE];
int gBattRecharge;
float gBattSoc;
int gLastStateOutputDischarge, gLastStateOutputCharge;
int gFunctionIndex;

// Initialwerte
gLastStateOutputDischarge = 0;
gLastStateOutputCharge = 0;

// Funktionen

int getStateOutputTimer(int aTimestamp)
{
	int lTimeValue = getcurrenttime()-aTimestamp;
	if (lTimeValue > STATE_OUTPUT_INTERVAL) {
		return 1;
	} else {
		return 0;
	}
}

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

void resetChargeControl()
{
	gFunctionIndex = FUNCTION_INDEX_RESET;
	setoutput(0,0);
	setoutput(1,0);
	setoutput(2,0);
	setoutputtext(0,"Keine Ladesteuerung");
}

void setBlockUncharge()
{
	gFunctionIndex = FUNCTION_INDEX_BLOCK;
	setoutput(0,2);
	setoutput(1,0);
	setoutput(2,0);
	setoutputtext(0,"Entladen Gesperrt");
}

void setChargeValue(float aValue)
{
	gFunctionIndex = FUNCTION_INDEX_CHARGE;
	setoutput(0,3);
	setoutput(1,getChargeValuePos(aValue));
	setoutput(2,getChargeValueNeg(aValue));
	gBuffer = "";
	sprintf(gBuffer,"Laden mit definierter Leistung: %f W",aValue);	
	if (getStateOutputTimer(gLastStateOutputCharge) > 0){
		setoutputtext(0,gBuffer);
	}
	gLastStateOutputCharge = getcurrenttime();
}

void setDischargeValue(float aValue)
{
	gFunctionIndex = FUNCTION_INDEX_DISCHARGE;
	setoutput(0,3);
	setoutput(1,getChargeValueNeg(aValue));
	setoutput(2,getChargeValuePos(aValue));
	gBuffer = "";
	sprintf(gBuffer,"Entladen mit definierter Leistung: %f W",aValue);
	if (getStateOutputTimer(gLastStateOutputDischarge) > 0){
		setoutputtext(0,gBuffer);
	}
	gLastStateOutputDischarge = getcurrenttime();
}

// Main Program
void MainCycle() {} //Empty Void Function for Index
while(TRUE)
{
	float lUnchargeProtectNt, lUnchargeProtectWb, lDifferenceCharge, lOwnUsage, lPvProduction;
	int lEnergyNt;
	lUnchargeProtectNt = getinput(0);
	lUnchargeProtectWb = getinput(1);
	lDifferenceCharge = getinput(2);
	lEnergyNt = getinput(9);
	lOwnUsage = getinput(8)*1000; // factor 1000 for conversion kw to w
	lPvProduction = getinput(7)*1000; // factor 1000 for conversion kw to w
	
	gBattSoc = getinput(11);
	
	// Check Battery low Soc Recharging
	if (gBattSoc < SOC_TH_RECHARGE_LOW)
	{
		gBattRecharge = 1;
	} else if (gBattSoc > SOC_TH_RECHARGE_HIGH)
	{
		gBattRecharge = 0;
	}

	// Choose Charging or Blocking mode by inputs
	if ((lUnchargeProtectNt > 0) && (lDifferenceCharge < 1))	{
		setBlockUncharge();
	} else if ((lUnchargeProtectWb > 0) && (lDifferenceCharge < 1)) {
		setDischargeValue(lOwnUsage);
	} else if ((lDifferenceCharge > 0) && (lPvProduction > 100)) {
		setChargeValue(lPvProduction - 100);
	} else if ((lDifferenceCharge > 0) && (lEnergyNt > 0)) {
		setChargeValue(500);
	} else {
		resetChargeControl();
	}
	
	if (gFunctionIndex != FUNCTION_INDEX_CHARGE) {
		gLastStateOutputCharge = 0;
	}
	if (gFunctionIndex != FUNCTION_INDEX_DISCHARGE) {
		gLastStateOutputDischarge = 0;
	}
}
