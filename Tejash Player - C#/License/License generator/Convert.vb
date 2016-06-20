
' Various conversions from one value to another.
' Hex conversions aren't actually used. =]

Module CConvert
    Public Function FromHex(ByVal Hex As String) As Long
        Return Long.Parse(Hex, Globalization.NumberStyles.HexNumber)
    End Function

    Public Function ToHex(ByVal Deci As Long) As String
        Return Deci.ToString("X")
    End Function

    Public Function FromBase36(ByVal IBase36 As String) As Long
        IBase36 = IBase36.ToUpper
        Dim Base36() As String = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}
        Dim i As Integer
        Dim v As Long
        For i = IBase36.Length - 1 To 0 Step i - 1
            Dim bc As Long = Math.Pow(36, ((IBase36.Length - 1) - i))
            If Base36.Contains(IBase36.Chars(i).ToString) Then
                v += Array.LastIndexOf(Base36, IBase36.Chars(i).ToString) * bc
            Else
                Throw New InvalidCastException
            End If
        Next
        Return v
    End Function

    Public Function ToBase36(ByVal IBase36 As Double) As String
        Dim Base36() As String = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"}
        Dim v As String
        Dim i As Decimal
        Do Until IBase36 < 1
            i = IBase36 Mod 36
            v = Base36(i) & v
            IBase36 = Math.DivRem(Long.Parse(IBase36), 36, Nothing)
        Loop
        Return v

    End Function
End Module
