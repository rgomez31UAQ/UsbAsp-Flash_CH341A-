unit buzzpirathw;

// By Dreg, Based from arduinohw

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, basehw, msgstr, utilfunc;

type

{ TBuzzpiratHardware }

TBuzzpiratHardware = class(TBaseHardware)
private
  FDevOpened: boolean;
  FStrError: string;
  FCOMPort: string;
public
  constructor Create;
  destructor Destroy; override;

  function GetLastError: string; override;
  function DevOpen: boolean; override;
  procedure DevClose; override;

  //spi
  function SPIRead(CS: byte; BufferLen: integer; var buffer: array of byte): integer; override;
  function SPIWrite(CS: byte; BufferLen: integer; buffer: array of byte): integer; override;
  function SPIInit(speed: integer): boolean; override;
  procedure SPIDeinit; override;

  //I2C
  procedure I2CInit; override;
  procedure I2CDeinit; override;
  function I2CReadWrite(DevAddr: byte;
                        WBufferLen: integer; WBuffer: array of byte;
                        RBufferLen: integer; var RBuffer: array of byte): integer; override;
  procedure I2CStart; override;
  procedure I2CStop; override;
  function I2CReadByte(ack: boolean): byte; override;
  function I2CWriteByte(data: byte): boolean; override; //return ack

  //MICROWIRE
  function MWInit(speed: integer): boolean; override;
  procedure MWDeinit; override;
  function MWRead(CS: byte; BufferLen: integer; var buffer: array of byte): integer; override;
  function MWWrite(CS: byte; BitsWrite: byte; buffer: array of byte): integer; override;
  function MWIsBusy: boolean; override;
end;

implementation

uses main;

constructor TBuzzpiratHardware.Create;
begin
  FHardwareName := 'Buzzpirat / Buspirate';
  FHardwareID := CHW_BUZZPIRAT;
end;

destructor TBuzzpiratHardware.Destroy;
begin
  DevClose;
end;

function TBuzzpiratHardware.GetLastError: string;
begin
    result := FStrError;
end;

function TBuzzpiratHardware.DevOpen: boolean;
var buff: byte;
    speed: cardinal;
begin
  if FDevOpened then DevClose;

  FDevOpened := false;

  FCOMPort := main.Buzzpirat_COMPort;

  if FCOMPort = '' then
  begin
    FStrError:= 'No port selected!';
    Exit(false);
  end;

  FDevOpened := true;
  Result := true;
end;

procedure TBuzzpiratHardware.DevClose;
begin
  FDevOpened := false;
end;


//SPI___________________________________________________________________________

function TBuzzpiratHardware.SPIInit(speed: integer): boolean;
var buff: byte;
begin
  if not FDevOpened then Exit(false);

  LogPrint('Not Implemented Yet');
  Exit(false);
end;

procedure TBuzzpiratHardware.SPIDeinit;
var buff: byte;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

function TBuzzpiratHardware.SPIRead(CS: byte; BufferLen: integer; var buffer: array of byte): integer;
var buff:  byte;
    bytes: integer;
const chunk = 64;
begin
  if not FDevOpened then Exit(-1);

  LogPrint('Not Implemented Yet');
  Exit(-1);
end;

function TBuzzpiratHardware.SPIWrite(CS: byte; BufferLen: integer; buffer: array of byte): integer;
var buff: byte;
    bytes: integer;
const chunk = 256;
begin
  if not FDevOpened then Exit(-1);

  LogPrint('Not Implemented Yet');
  Exit(-1);
end;

//i2c___________________________________________________________________________

procedure TBuzzpiratHardware.I2CInit;
var
  khz: integer;
  pullups: integer;
  power: integer;
  just_i2c_scanner: integer;
begin
  if not FDevOpened then Exit;

  just_i2c_scanner := 0;
  khz := 0;
  pullups := 0;
  power := 0;

  if MainForm.MenuBuzzpiratI2C5KHz.Checked then
  begin
       LogPrint('5khz');
       khz := 5;
  end
  else if MainForm.MenuBuzzpiratI2C50KHz.Checked then
  begin
       LogPrint('50khz');
       khz := 50;
  end
  else if MainForm.MenuBuzzpiratI2C100KHz.Checked then
  begin
       LogPrint('100khz');
       khz := 100;
  end;

  if MainForm.MenuBuzzpiratPower.Checked then
  begin
       LogPrint('Power ON');
       power := 1;
  end;

  if MainForm.MenuBuzzpiratPullups.Checked then
  begin
       LogPrint('Pull-ups ON');
       pullups := 1;
  end;

  if MainForm.MenuBuzzpiratJustI2CScan.Checked then
  begin
       LogPrint('JUST I2C SCANNER');
       just_i2c_scanner := 1;
  end;

  LogPrint('Not Implemented Yet');
  Exit;
end;

procedure TBuzzpiratHardware.I2CDeinit;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

function TBuzzpiratHardware.I2CReadWrite(DevAddr: byte;
                        WBufferLen: integer; WBuffer: array of byte;
                        RBufferLen: integer; var RBuffer: array of byte): integer;
var
  StopAfterWrite: byte;
  buff: byte;
  bytes: integer;
const rchunk = 64;
      wchunk = 256;
begin
  if not FDevOpened then Exit(-1);

  LogPrint('Not Implemented Yet');
  Exit(-1);
end;

procedure TBuzzpiratHardware.I2CStart;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

procedure TBuzzpiratHardware.I2CStop;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

function TBuzzpiratHardware.I2CReadByte(ack: boolean): byte;
var
  Status: byte;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

function TBuzzpiratHardware.I2CWriteByte(data: byte): boolean;
var
  Status: byte;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

//MICROWIRE_____________________________________________________________________

function TBuzzpiratHardware.MWInit(speed: integer): boolean;
var buff: byte;
begin
  if not FDevOpened then Exit(false);

  LogPrint('Not Implemented Yet');
  Exit(false);
end;

procedure TBuzzpiratHardware.MWDeInit;
var buff: byte;
begin
  if not FDevOpened then Exit;

  LogPrint('Not Implemented Yet');
  Exit;
end;

function TBuzzpiratHardware.MWRead(CS: byte; BufferLen: integer; var buffer: array of byte): integer;
var buff:  byte;
    bytes: integer;
const chunk = 64;
begin
  if not FDevOpened then Exit(-1);

  LogPrint('Not Implemented Yet');
  Exit(-1);
end;

function TBuzzpiratHardware.MWWrite(CS: byte; BitsWrite: byte; buffer: array of byte): integer;
var buff: byte;
    bytes: byte;
const chunk = 32;
begin
  if not FDevOpened then Exit(-1);

  LogPrint('Not Implemented Yet');
  Exit(-1);
end;

function TBuzzpiratHardware.MWIsBusy: boolean;
var
  buff: byte;
begin
  LogPrint('Not Implemented Yet');
  Exit(false);
end;




end.

