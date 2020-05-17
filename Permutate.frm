VERSION 5.00
Begin VB.Form frmMain 
   BorderStyle     =   1  'Fixed Single
   Caption         =   "Permutate"
   ClientHeight    =   4395
   ClientLeft      =   45
   ClientTop       =   330
   ClientWidth     =   5655
   LinkTopic       =   "Form1"
   MaxButton       =   0   'False
   MinButton       =   0   'False
   ScaleHeight     =   4395
   ScaleWidth      =   5655
   StartUpPosition =   1  'CenterOwner
   Begin VB.CheckBox chkRepeat 
      Caption         =   "&Repeat"
      Height          =   315
      Left            =   3720
      TabIndex        =   3
      Top             =   60
      Width           =   855
   End
   Begin VB.TextBox txtOutput 
      Height          =   3915
      Left            =   60
      Locked          =   -1  'True
      MultiLine       =   -1  'True
      ScrollBars      =   2  'Vertical
      TabIndex        =   2
      Top             =   420
      Width           =   5535
   End
   Begin VB.CommandButton cmdPermutate 
      Caption         =   "&Permutate"
      Default         =   -1  'True
      Height          =   315
      Left            =   4680
      TabIndex        =   1
      Top             =   60
      Width           =   915
   End
   Begin VB.TextBox txtWord 
      Height          =   285
      Left            =   60
      TabIndex        =   0
      Text            =   "blade"
      ToolTipText     =   "Enter a word here; try 5 characters or more if you have a GOOD CPU!!!"
      Top             =   60
      Width           =   3555
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' Permutation of letters in variable length words (recursive method).
' Programmed and designed by Samuel Gomes, 2000.

Option Explicit

Private word As String, myWord As String
Private wordSize As Integer, permutationCount As Long
Private letter() As String * 1
Private counter() As Integer
Private badCounter() As Integer

Sub MyPrint(ByVal s As String)
    txtOutput.Text = txtOutput.Text & s
End Sub

Function BadCounters() As Boolean
    Dim i As Integer, j As Integer, x As Integer

    For i = 1 To wordSize
        badCounter(i) = False
    Next

    For i = 1 To wordSize - 1
        For j = i + 1 To wordSize
            If (counter(i) = counter(j)) Then
                badCounter(i) = True
            End If
        Next
    Next

    For x = 1 To wordSize
        If (badCounter(x)) Then
            BadCounters = True
            Exit Function
        End If
    Next
End Function

Sub FixWord()
    Dim i As Integer, j As Integer, repeated As Boolean
    Dim rep As String * 1
    
    txtOutput.Text = ""
    MyPrint "Permutate. Version 2.2" & vbCrLf
    MyPrint "Copyright (c) Samuel Gomes (Blade), 1999-2003" & vbCrLf
    MyPrint "mailto: blade_go@hotmail.com" & vbCrLf
    MyPrint vbCrLf
    MyPrint "Algorithm: Recursive, Extended" & vbCrLf

    word = txtWord.Text
    myWord = ""
    permutationCount = 0
    
    If (chkRepeat.Value = vbChecked) Then
        myWord = word
    Else
        For i = 1 To Len(word)
            For j = 1 To Len(myWord)
                If (Mid(myWord, j, 1) = Mid(word, i, 1)) Then repeated = True
            Next

            If Not (repeated) Then myWord = myWord + Mid(word, i, 1)
            repeated = False
        Next
    End If

    wordSize = Len(myWord)
End Sub

Sub PermutateString(letterIndex As Integer)
    Dim i As Integer, x As Integer

    For i = 1 To wordSize
        counter(letterIndex) = i
        If (letterIndex = wordSize) Then
            If Not (BadCounters) Then
                permutationCount = permutationCount + 1
                For x = 1 To wordSize
                    MyPrint letter(counter(x))
                Next
                MyPrint vbTab
            End If
        End If
        If (letterIndex < wordSize) Then PermutateString letterIndex + 1
    Next
End Sub

Private Sub cmdPermutate_Click()
    Dim i As Integer
    
    FixWord
    
    ReDim letter(1 To wordSize) As String * 1
    ReDim counter(1 To wordSize) As Integer
    ReDim badCounter(1 To wordSize) As Integer
    
    ' Copy each letter in the array "letter"
    For i = 1 To wordSize
        letter(i) = Mid(myWord, i, 1)
    Next

    MyPrint vbCrLf
    MyPrint "Permutations of the letters of the word: " & word & " (" & myWord & ")" & vbCrLf
    MyPrint vbCrLf

    Screen.MousePointer = vbHourglass
    PermutateString 1
    Screen.MousePointer = vbDefault

    MyPrint vbCrLf & vbCrLf
    MyPrint "No. of permutations performed = " & permutationCount & vbCrLf
End Sub

Private Sub Form_Load()
    txtWord_Change
End Sub

Private Sub txtWord_Change()
    cmdPermutate.Enabled = (txtWord.Text <> "")
End Sub
