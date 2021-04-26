# ATTENTION : Penser à donner les droits d'utiliser les scripts en local via 
#             cette commande directement dans la console powershell
#             set-executionpolicy remotesigned
Param(
  [Parameter(Mandatory=$true, Position=0)][String] $filename,
  [Parameter(Mandatory=$true, Position=1)][String] $pulsesStr
) 

$errorActionPreference="Stop"

# if ([string]::IsNullOrEmpty($args[0])) { throw "Le nom de fichier est obligatoire" }
# if ([string]::IsNullOrEmpty($args[1])) { throw "Les pulses sont obligatoires" }
# $filename = $args[0];
# $pulses = $args[1]

$pulses = $pulsesStr.Split(',')

Remove-Item $filename -ErrorAction Ignore

# header
Out-File -append -Encoding ASCII -filePath $filename -inputObject (("0`r`n") * 5000)

# pulses
$nextBit = 1;
foreach ($pulse in $pulses)
{
  Out-File -append -Encoding ASCII -filePath $filename -inputObject (("$nextBit`r`n") * $pulse)
  $nextBit = 1 - $nextBit
}

# trailer
if ($nextBit = 1)
{
  $trailernoise = "1`r`n0";
}
else
{
  $trailernoise = "0`r`n1";
}
  
Out-File -append -Encoding ASCII -filePath $filename -inputObject (("$trailernoise`r`n") * 2500)

& "C:\Program Files (x86)\sigrok\PulseView\pulseview.exe" -i "$filename" -I csv:samplerate=1000000


