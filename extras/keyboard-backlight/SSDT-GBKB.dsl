/*
 * Ponte verso \_SB.SCAI (SAM0429) per GalaxyBookKbd su macOS.
 * AppleACPIPlatform esegue solo metodi con nome semplice relativo al nodo:
 * questo dispositivo espone CSFI e due letture diagnostiche che inoltrano a SCAI.
 * Visibile solo su macOS (_OSI "Darwin"); non modifica oggetti esistenti.
 */
DefinitionBlock ("", "SSDT", 2, "VIK", "GBKB", 0x00001000)
{
    External (_SB_.SCAI, DeviceObj)
    External (_SB_.SCAI.CSFI, MethodObj)
    External (_SB_.SCAI._STA, MethodObj)
    External (OSYS, FieldUnitObj)

    Scope (_SB)
    {
        Device (GBKB)
        {
            Name (_HID, "GBKB0000")
            Method (_STA, 0, NotSerialized)
            {
                If (_OSI ("Darwin"))
                {
                    Return (0x0F)
                }
                Return (Zero)
            }

            Method (CSFI, 1, Serialized)
            {
                Return (\_SB.SCAI.CSFI (Arg0))
            }

            Method (GOSY, 0, Serialized)
            {
                Return (\OSYS)
            }

            Method (GSTA, 0, Serialized)
            {
                Return (\_SB.SCAI._STA ())
            }
        }
    }
}
