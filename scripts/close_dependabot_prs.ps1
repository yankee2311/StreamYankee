<#
.SYNOPSIS
    Close all Dependabot Pull Requests in the repository
.DESCRIPTION
    This script closes all open Dependabot PRs using GitHub API
.EXAMPLE
    .\close_dependabot_prs.ps1 -GitHubToken "ghp_xxx"
.PARAMETER GitHubToken
    Your GitHub Personal Access Token with 'repo' permissions
.PARAMETER RepoOwner
    Repository owner (default: yankee2311)
.PARAMETER RepoName
    Repository name (default: StreamYankee)
#>

param(
    [Parameter(Mandatory=$true)]
    [string]$GitHubToken,
    
    [string]$RepoOwner = "yankee2311",
    [string]$RepoName = "StreamYankee"
)

$bearerToken = "Bearer $GitHubToken"
$headers = @{
    "Authorization" = $bearerToken
    "Accept" = "application/vnd.github+json"
    "X-GitHub-Api-Version" = "2022-11-28"
}

Write-Host "Fetching open PRs from $RepoOwner/$RepoName..." -ForegroundColor Cyan

$page = 1
$allPRs = @()

do {
    $url = "https://api.github.com/repos/$RepoOwner/$RepoName/pulls?state=open&page=$page&per_page=100"
    $response = Invoke-RestMethod -Uri $url -Headers $headers -Method Get
    $allPRs += $response
    $page++
} while ($response.Count -eq 100)

$dependabotPRs = $allPRs | Where-Object { $_.user.login -eq "dependabot[bot]" }

Write-Host "Found $($dependabotPRs.Count) Dependabot PRs to close" -ForegroundColor Yellow

foreach ($pr in $dependabotPRs) {
    Write-Host "Closing PR #$($pr.number): $($pr.title)" -ForegroundColor Cyan
    
    $updateUrl = "https://api.github.com/repos/$RepoOwner/$RepoName/pulls/$($pr.number)"
    $body = @{
        state = "closed"
    } | ConvertTo-Json
    
    try {
        Invoke-RestMethod -Uri $updateUrl -Headers $headers -Method Patch -Body $body | Out-Null
        Write-Host "  [OK] PR #$($pr.number) closed" -ForegroundColor Green
    }
    catch {
        Write-Host "  [ERROR] Failed to close PR #$($pr.number): $_" -ForegroundColor Red
    }
    
    Start-Sleep -Milliseconds 500
}

Write-Host "`nCompleted! Closed $($dependabotPRs.Count) Dependabot PRs" -ForegroundColor Green